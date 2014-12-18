#include <QByteArray>
#include <QBuffer>
#include <QQueue>
#include <QTcpSocket>
#include "qcorconnection.h"
#include "corprotocol.h"

///////////////////////////////////////////////////////////////////////
// Helper Objects
///////////////////////////////////////////////////////////////////////

class SendQueueItem
{
public:
  SendQueueItem() : frame(0), device(0) {}
  QCorFrame *frame;
  QIODevice *device;
};

///////////////////////////////////////////////////////////////////////
// Private class
///////////////////////////////////////////////////////////////////////

class QCorConnection::Private
{
public:
  QCorConnection *owner;
  QTcpSocket *socket;

  QByteArray buffer; ///< Incoming data buffer.
  QCorFrameRefPtr frame; ///< Current incoming frame.

  QQueue<SendQueueItem*> sendQueue;
  SendQueueItem *sendQueueCurrent;

  cor_parser_settings corSettings;
  cor_parser *corParser;
  cor_frame::correlation_t nextCorrelationId;

  static int onParserFrameBegin(cor_parser *parser)
  {
    QCorConnection *conn = static_cast<QCorConnection*>(parser->object);
    conn->d->frame.reset(new QCorFrame(conn));
    conn->d->frame->_state = QCorFrame::TransferingState;
    return 0;
  }

  static int onParserFrameHeaderBegin(cor_parser *parser)
  {
    return 0;
  }

  static int onParserFrameHeaderEnd(cor_parser *parser)
  {
    QCorConnection *conn = static_cast<QCorConnection*>(parser->object);
    if (conn->d->frame) {
      switch (parser->request->type) {
        case cor_frame::TYPE_REQUEST:
          conn->d->frame->_type = QCorFrame::RequestType;
          break;
        case cor_frame::TYPE_RESPONSE:
          conn->d->frame->_type = QCorFrame::ResponseType;
          break;
      }
      //emit conn->newFrame(conn->d->frame);
    }
    return 0;
  }

  static int onParserFrameBodyData(cor_parser *parser, const uint8_t *data, size_t length) // TODO Do we need to delete "data"?
  {
    QCorConnection *conn = static_cast<QCorConnection*>(parser->object);
    if (conn->d->frame) {
      conn->d->frame->_data.append(QByteArray((const char*)data, length));
      //conn->d->frame->device()->write((const char *)data, length);
      //emit conn->d->frame->newBodyData(QByteArray((const char*)data, length));
    }
    return 0;
  }

  static int onParserFrameEnd(cor_parser *parser)
  {
    QCorConnection *conn = static_cast<QCorConnection*>(parser->object);
    QCorFrameRefPtr f = conn->d->frame;
    conn->d->frame.clear();
    emit conn->newFrame(f);
    //emit conn->d->frame->end();
    return 0;
  }
};

///////////////////////////////////////////////////////////////////////

QCorConnection::QCorConnection(QObject *parent) :
  QObject(parent),
  d(new Private())
{
  d->owner = this;
  d->socket = 0;
  d->frame.clear();
  d->nextCorrelationId = 0;

  qRegisterMetaType<QCorFrameRefPtr>("QCorFrameRefPtr");

  cor_parser_settings_init(d->corSettings);
  d->corSettings.on_frame_begin = &(Private::onParserFrameBegin);
  d->corSettings.on_frame_header_begin = &(Private::onParserFrameHeaderBegin);
  d->corSettings.on_frame_header_end = &(Private::onParserFrameHeaderEnd);
  d->corSettings.on_frame_body_data = &(Private::onParserFrameBodyData);
  d->corSettings.on_frame_end = &(Private::onParserFrameEnd);

  d->corParser = (cor_parser *)malloc(sizeof(cor_parser));
  cor_parser_init(d->corParser);
  d->corParser->object = this;
}

QCorConnection::~QCorConnection()
{
  delete d->socket;
  d->frame.clear();
  free(d->corParser);
  delete d;
}

QAbstractSocket::SocketState QCorConnection::state() const
{
  return d->socket ? d->socket->state() : QAbstractSocket::UnconnectedState;
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
}

void QCorConnection::sendTestRequest()
{
  cor_frame frame;
  frame.version = 1;
  frame.type = cor_frame::TYPE_REQUEST;
  frame.flags = 0;
  frame.correlation_id = 0;
  frame.length = 1024 * 4;
  frame.data = new uint8_t[frame.length];
  for (cor_frame::data_length_t i = 0; i < frame.length; ++i) {
    frame.data[i] = 8;
  }
  d->socket->write((const char *)&frame.version, sizeof(frame.version));
  d->socket->write((const char *)&frame.type, sizeof(frame.type));
  d->socket->write((const char *)&frame.flags, sizeof(frame.flags));
  d->socket->write((const char *)&frame.correlation_id, sizeof(frame.correlation_id));
  d->socket->write((const char *)&frame.length, sizeof(frame.length));
  d->socket->write((const char *)&frame.data, frame.length);
  delete frame.data;
}

void QCorConnection::sendResponse(QCorResponse *res, QIODevice *dev)
{

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
    deleteLater();
    if (d->frame && !d->frame->state() != QCorFrame::FinishedState) {
      d->frame->_state = QCorFrame::ErrorState;
      //emit d->frame->end();
    }
    break;
  }
}

void QCorConnection::doNextSendItem()
{
  if (d->sendQueueCurrent || d->sendQueue.count() == 0) {
    return;
  }
  d->sendQueueCurrent = d->sendQueue.dequeue();
  QObject::connect(d->sendQueueCurrent->device, SIGNAL(aboutToClose()), this, SLOT(onCurrentSendItemDone()));
}

void QCorConnection::onCurrentSendItemDone()
{
  delete d->sendQueueCurrent;
  d->sendQueueCurrent = 0;
  doNextSendItem();
}