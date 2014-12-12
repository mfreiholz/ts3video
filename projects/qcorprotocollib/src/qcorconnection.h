#ifndef QCORCONNECTION_HEADER
#define QCORCONNECTION_HEADER

#include <QObject>
#include <QTcpSocket>
class QCorRequest;

class QCorConnection : public QObject
{
  Q_OBJECT

public:
  QCorConnection(QTcpSocket *socket, QObject *parent);
  QTcpSocket* socket() const;

private slots:
  void onSocketReadyRead();
  void onSocketStateChanged(QAbstractSocket::SocketState state);

signals:
  void newRequest(QCorRequest *request);

private:
  QTcpSocket *_socket;
  QCorRequest *_currentRequest;
};

#endif