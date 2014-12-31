#ifndef CONTROLCONNECTIONHANDLER_HEADER
#define CONTROLCONNECTIONHANDLER_HEADER

#include <QObject>
class QTcpSocket;
class ControlServer;

class ControlConnectionHandler : public QObject
{
  Q_OBJECT

public:
  ControlConnectionHandler(ControlServer *server, QTcpSocket *socket, QObject *parent = 0);
  virtual ~ControlConnectionHandler();
  QTcpSocket* socket() const;

signals:
  void disconnected();

private:
  class Private;
  Private *d;
};

#endif