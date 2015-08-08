#ifndef TS3SERVERQUERYCLIENTCONNECTION_H
#define TS3SERVERQUERYCLIENTCONNECTION_H

#include <QTcpSocket>
#include <QHostAddress>

/*
  Command order:
    login <loginname> <password>
    use port=<serverport>
    clientupdate nickname=<new-nickname>
    <...>
    quit
*/
class TS3ServerQueryClientConnection : public QObject
{
  Q_OBJECT

public:
  TS3ServerQueryClientConnection(QObject *parent);
  void init();

private slots:
  void onSocketConnected();
  void onSocketDisconnected();
  void onSocketError(QAbstractSocket::SocketError err);
  void onSocketReadyRead();

private:
  QHostAddress _address;
  qint16 _port;

  qint16 _virtualServerPort;
  QString _clientLoginName;
  QString _clientLoginPassword;

  QTcpSocket* _socket;
  QByteArray _buffer;
};


class TS3ServerQueryCommand : public QObject
{
  Q_OBJECT

public:
  TS3ServerQueryCommand(QObject *parent);
};

#endif