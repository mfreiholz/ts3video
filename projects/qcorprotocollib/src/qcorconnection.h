#ifndef QCORCONNECTION_HEADER
#define QCORCONNECTION_HEADER

#include <QObject>
#include <QByteArray>
#include <QQueue>
#include <QTcpSocket>
class QCorFrame;

class QCorConnection : public QObject
{
  Q_OBJECT
  class Private;
  Private *d;

public:
  QCorConnection(QObject *parent);
  virtual ~QCorConnection();

public slots:
  void connectWith(quintptr descriptor);
  void connectTo(const QHostAddress &address, quint16 port);
  void send(QCorFrame *frame);

private slots:
  void onSocketReadyRead();
  void onSocketStateChanged(QAbstractSocket::SocketState state);

private slots:
  void sendNext();
  void onSendNextDone();

signals:
  /* Emits for every new frame.
   * Note: The frame has to be deleted by receiver with deleteLater().
   * It doesn't have a parent object.
   */
  void newFrame(QCorFrame *frame);

private:
  QTcpSocket *_socket;
  QByteArray _buffer;
  QCorFrame *_frame;

  QQueue<QCorFrame*> _sendQueue;
  QCorFrame *_sendFrame;
};

#endif