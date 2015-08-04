#ifndef TS3SERVERBRIDGE_H
#define TS3SERVERBRIDGE_H

#include <QObject>

class VirtualServer;

class TS3ServerBridge : public QObject
{
  Q_OBJECT

public:
  TS3ServerBridge(VirtualServer* server, QObject* parent);
  bool init();

private:
  VirtualServer* _server;
};

#endif