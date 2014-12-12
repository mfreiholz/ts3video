#include "qcorconnection.h"
#include "qcorframe.h"
#include "corprotocol.h"

///////////////////////////////////////////////////////////////////////
// Private class
///////////////////////////////////////////////////////////////////////

class QCorConnection::Private
{
public:
  cor_parser_settings corSettings;
  cor_parser *corParser;

  static int onParserFrameBegin(cor_parser *parser)
  {
    QCorConnection *conn = static_cast<QCorConnection*>(parser->object);
    conn->_frame = new QCorFrame(conn, 0);
    emit conn->newFrame(conn->_frame);
    return 0;
  }
  
  static int onParserFrameBodyData(cor_parser *parser, const uint8_t *data, size_t length) // TODO Do we need to delete "data"?
  {
    QCorConnection *conn = static_cast<QCorConnection*>(parser->object);
    if (conn->_frame) {
      emit conn->_frame->newBodyData(QByteArray((const char*)data, length));
    }
    return 0;
  }
  
  static int onParserFrameEnd(cor_parser *parser)
  {
    QCorConnection *conn = static_cast<QCorConnection*>(parser->object);
    if (conn->_frame) {
      conn->_frame->setState(QCorFrame::FinishedState);
      emit conn->_frame->end();
      conn->_frame = 0;
    }
    return 0;
  }
};

///////////////////////////////////////////////////////////////////////

QCorConnection::QCorConnection(QTcpSocket *socket, QObject *parent) :
  QObject(parent),
  d(new Private()),
  _socket(socket),
  _frame(0)
{
  d->corSettings.on_frame_begin = &(Private::onParserFrameBegin);
  d->corSettings.on_frame_body_data = &(Private::onParserFrameBodyData);
  d->corSettings.on_frame_end = &(Private::onParserFrameEnd);

  d->corParser = (cor_parser *)malloc(sizeof(cor_parser));
  cor_parser_init(d->corParser);
  d->corParser->object = this;

  connect(socket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
  connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
}

QCorConnection::~QCorConnection()
{
  free(d->corParser);
  delete d;
  delete _socket;
}

void QCorConnection::write(const QByteArray &data)
{
  _socket->write(data);
}

void QCorConnection::onSocketReadyRead()
{
  quint64 available = 0;
  while ((available = _socket->bytesAvailable()) > 0) {
    _buffer.append(_socket->read(available));
    const size_t read = cor_parser_parse(d->corParser, d->corSettings, (uint8_t*)_buffer.constData(), _buffer.size());
    if (read > 0) {
      if (read < _buffer.size()) {
        _buffer = _buffer.mid(read);
      } else {
        _buffer.clear();
      }
    }
  }
}

void QCorConnection::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
  case QAbstractSocket::UnconnectedState:
    deleteLater();
    if (_frame && !_frame->state() != QCorFrame::FinishedState) {
      _frame->setState(QCorFrame::ErrorState);
      _frame->end();
    }
    break;
  }
}
