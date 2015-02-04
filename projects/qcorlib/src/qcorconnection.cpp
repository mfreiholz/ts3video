#include <QTcpSocket>
#include "qcorconnection.h"
#include "qcorconnection_p.h"
#include "qcorreply.h"

///////////////////////////////////////////////////////////////////////

QCorConnection::QCorConnection(QObject *parent) :
  QObject(parent),
  d(new QCorConnectionPrivate(this))
{
  d->owner = this;
  d->socket = 0;
  d->frame.clear();
  d->nextCorrelationId = 0;
  d->sendQueueCurrent = 0;

  qRegisterMetaType<QCorFrameRefPtr>("QCorFrameRefPtr");

  cor_parser_settings_init(d->corSettings);
  d->corSettings.on_frame_begin = &(QCorConnectionPrivate::onParserFrameBegin);
  d->corSettings.on_frame_header_begin = &(QCorConnectionPrivate::onParserFrameHeaderBegin);
  d->corSettings.on_frame_header_end = &(QCorConnectionPrivate::onParserFrameHeaderEnd);
  d->corSettings.on_frame_body_data = &(QCorConnectionPrivate::onParserFrameBodyData);
  d->corSettings.on_frame_end = &(QCorConnectionPrivate::onParserFrameEnd);

  d->corParser = (cor_parser *)malloc(sizeof(cor_parser));
  cor_parser_init(d->corParser);
  d->corParser->object = d;
}

QCorConnection::~QCorConnection()
{
  d->owner = 0;
  delete d->socket;
  d->buffer.clear();
  d->frame.clear();
  free(d->corParser);
  d->nextCorrelationId = 0;
  qDeleteAll(d->sendQueue);
  delete d->sendQueueCurrent;
  qDeleteAll(d->replies);
  delete d;
}

QAbstractSocket::SocketState QCorConnection::state() const
{
  return d->socket ? d->socket->state() : QAbstractSocket::UnconnectedState;
}

QTcpSocket *QCorConnection::socket() const
{
  return d->socket;
}

void QCorConnection::connectWith(quintptr descriptor)
{
  if (d->socket) {
    d->socket->abort();
    delete d->socket;
  }
  d->socket = new QTcpSocket(this);
  d->socket->setSocketDescriptor(descriptor);
  d->socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
  d->socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
  onSocketStateChanged(QAbstractSocket::ConnectedState);
  connect(d->socket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
  connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
  connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(stateChanged(QAbstractSocket::SocketState)));
  connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(error(QAbstractSocket::SocketError)));
}

void QCorConnection::connectWith(QTcpSocket *socket)
{
  if (d->socket) {
    d->socket->abort();
    delete d->socket;
  }
  d->socket = socket;
  d->socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
  d->socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
  onSocketStateChanged(QAbstractSocket::ConnectedState);
  connect(d->socket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
  connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
  connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(stateChanged(QAbstractSocket::SocketState)));
  connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(error(QAbstractSocket::SocketError)));
}

void QCorConnection::connectTo(const QHostAddress &address, quint16 port)
{
  if (d->socket) {
    d->socket->abort();
    delete d->socket;
  }
  d->socket = new QTcpSocket(this);
  d->socket->connectToHost(address, port);
  d->socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
  d->socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
  connect(d->socket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
  connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
  connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(stateChanged(QAbstractSocket::SocketState)));
  connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(error(QAbstractSocket::SocketError)));
}

void QCorConnection::disconnectFromHost()
{
  if (!d->socket) {
    return;
  }
  d->socket->disconnectFromHost();
}

QCorReply* QCorConnection::sendRequest(const QCorFrame &frame)
{
  if (d->socket->state() != QAbstractSocket::ConnectedState) {
    return 0;
  }

  SendQueueItem *item = new SendQueueItem();
  item->frame.version = 1;
  item->frame.type = cor_frame::TYPE_REQUEST;
  item->frame.correlation_id = ++d->nextCorrelationId;
  item->frame.length = frame.data().size();
  item->data = frame.data();
  d->sendQueue.enqueue(item);

  ReplyItem *rep = new ReplyItem();
  rep->res = new QCorReply(this);
  rep->dtEnqueued = QDateTime::currentDateTimeUtc();
  d->replies[item->frame.correlation_id] = rep;

  doNextSendItem();
  return rep->res;
}

void QCorConnection::sendResponse(const QCorFrame &frame)
{
  if (d->socket->state() != QAbstractSocket::ConnectedState) {
    return;
  }

  SendQueueItem *item = new SendQueueItem();
  item->frame.version = 1;
  item->frame.type = cor_frame::TYPE_RESPONSE;
  item->frame.correlation_id = frame.correlationId();
  item->frame.length = frame.data().size();
  item->data = frame.data();
  d->sendQueue.enqueue(item);
  doNextSendItem();
}

void QCorConnection::onSocketReadyRead()
{
  quint64 available = 0;
  while ((available = d->socket->bytesAvailable()) > 0) {
    d->buffer.append(d->socket->read(available));
    const size_t read = cor_parser_parse(d->corParser, d->corSettings, (uint8_t*)d->buffer.constData(), d->buffer.size());
    if (read > 0) {
      if (read < d->buffer.size()) {
        d->buffer = d->buffer.mid(read);
      } else {
        d->buffer.clear();
      }
    }
  }
}

void QCorConnection::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::UnconnectedState:
      //deleteLater();
      if (d->frame && !d->frame->state() != QCorFrame::FinishedState) {
        d->frame->setState(QCorFrame::ErrorState);
      }
      break;
  }
}

void QCorConnection::doNextSendItem()
{
  if (d->socket->state() != QAbstractSocket::ConnectedState) {
    return;
  }

  if (d->sendQueueCurrent || d->sendQueue.count() == 0) {
    return;
  }
  d->sendQueueCurrent = d->sendQueue.dequeue();
  
  // Write the actual request here now...
  // Write frame in network byte order.
  const cor_frame frame = d->sendQueueCurrent->frame;

  //d->socket->write((const char *)&frame.version, sizeof(frame.version));
  //d->socket->write((const char *)&frame.type, sizeof(frame.type));
  //d->socket->write((const char *)&frame.flags, sizeof(frame.flags));
  //d->socket->write((const char *)&frame.correlation_id, sizeof(frame.correlation_id));
  //d->socket->write((const char *)&frame.length, sizeof(frame.length));
  //d->socket->write(d->sendQueueCurrent->data);

  QDataStream out(d->socket);
  out.setByteOrder(QDataStream::BigEndian);
  out << frame.version;
  out << frame.type;
  out << frame.flags;
  out << frame.correlation_id;
  out << frame.length;
  out.writeRawData(d->sendQueueCurrent->data.constData(), frame.length);

  QMetaObject::invokeMethod(this, "onCurrentSendItemDone", Qt::QueuedConnection);
}

void QCorConnection::onCurrentSendItemDone()
{
  delete d->sendQueueCurrent;
  d->sendQueueCurrent = 0;
  doNextSendItem();
}

///////////////////////////////////////////////////////////////////////
// Private class
///////////////////////////////////////////////////////////////////////

QCorConnectionPrivate::QCorConnectionPrivate(QCorConnection *owner) :
  owner(owner)
{
}

int QCorConnectionPrivate::onParserFrameBegin(cor_parser *parser)
{
  QCorConnectionPrivate *d = static_cast<QCorConnectionPrivate*>(parser->object);
  d->frame.reset(new QCorFrame());
  d->frame->setState(QCorFrame::TransferingState);
  return 0;
}

int QCorConnectionPrivate::onParserFrameHeaderBegin(cor_parser *parser)
{
  return 0;
}

int QCorConnectionPrivate::onParserFrameHeaderEnd(cor_parser *parser)
{
  QCorConnectionPrivate *d = static_cast<QCorConnectionPrivate*>(parser->object);
  if (d->frame) {
    switch (parser->request->type) {
      case cor_frame::TYPE_REQUEST:
        d->frame->setType(QCorFrame::RequestType);
        break;
      case cor_frame::TYPE_RESPONSE:
        d->frame->setType(QCorFrame::ResponseType);
        break;
    }
    d->frame->setCorrelationId(parser->request->correlation_id);
  }
  return 0;
}

int QCorConnectionPrivate::onParserFrameBodyData(cor_parser *parser, const uint8_t *data, size_t length) // TODO Do we need to delete "data"?
{
  QCorConnectionPrivate *d = static_cast<QCorConnectionPrivate*>(parser->object);
  if (d->frame) {
    d->frame->appendData(QByteArray((const char*)data, length));
  }
  return 0;
}

int QCorConnectionPrivate::onParserFrameEnd(cor_parser *parser)
{
  QCorConnectionPrivate *d = static_cast<QCorConnectionPrivate*>(parser->object);
  QCorFrameRefPtr f = d->frame;
  d->frame.clear();

  // Check "replies" and notify associated QCorResponse object.
  switch (parser->request->type) {
  case cor_frame::TYPE_REQUEST:
    emit d->owner->newIncomingRequest(f);
    break;
  case cor_frame::TYPE_RESPONSE:
    ReplyItem *rep = d->replies.take(parser->request->correlation_id);
    if (rep) {
      rep->dtReceived = QDateTime::currentDateTimeUtc();
      rep->res->setFrame(f);
      rep->res->setElapsedMillis(rep->dtEnqueued.msecsTo(rep->dtReceived));
      emit rep->res->finished();
      delete rep;
    }
    break;
  }
  return 0;
}