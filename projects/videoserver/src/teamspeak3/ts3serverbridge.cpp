#include "ts3serverbridge.h"

TS3ServerBridge::TS3ServerBridge(VirtualServer* server, QObject* parent) :
  QObject(parent),
  _server(server)
{

}

bool TS3ServerBridge::init()
{
  return true;
}