#include "ts3serverbridge.h"
#include "humblelogging/api.h"

HUMBLE_LOGGER(HL, "ts3");

TS3ServerBridge::TS3ServerBridge(VirtualServer* server, QObject* parent) :
  QObject(parent),
  _server(server)
{

}

bool TS3ServerBridge::init()
{
  return true;
}