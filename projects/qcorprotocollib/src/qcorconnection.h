#ifndef QCORCONNECTION_HEADER
#define QCORCONNECTION_HEADER

#include <QObject>
#include <QByteArray>
#include <QTcpSocket>
class QCorFrame;

class QCorConnection : public QObject
{
  Q_OBJECT

public:
  QCorConnection(QTcpSocket *socket, QObject *parent);
  virtual ~QCorConnection();
  void write(const QByteArray &data);

private slots:
  void onSocketReadyRead();
  void onSocketStateChanged(QAbstractSocket::SocketState state);

signals:
  /* Emits for every new frame.
   * Note: The frame has to be deleted by receiver with deleteLater().
   */
  void newFrame(QCorFrame *frame);

private:
  class Private;
  Private *d;
  QTcpSocket *_socket;
  QByteArray _buffer;
  QCorFrame *_frame;
};

#endif