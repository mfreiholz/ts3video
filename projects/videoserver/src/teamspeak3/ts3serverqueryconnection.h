#ifndef TS3SERVERQUERYCLIENTCONNECTION_H
#define TS3SERVERQUERYCLIENTCONNECTION_H

#include <QTcpSocket>
#include <QFuture>

/*
  Command order:
    login <loginname> <password>
    use port=<serverport>
    clientupdate nickname=<new-nickname>

    ...

    quit
*/
class TS3ServerQueryClientConnection : public QObject
{
  Q_OBJECT

public:
  TS3ServerQueryClientConnection(QObject *parent);
  void init();

private slots:
  void onStateChanged(QAbstractSocket::SocketState state);
  void onReadyRead();

private:
  QString _serverAddress;
  qint16 _serverPort;
  qint16 _serverQueryPort;
  QString _loginName;
  QString _loginPassword;

  QTcpSocket* _socket;
  QByteArray _buffer;
};

#endif