#include "clientconnectionhandler.h"

#include "ts3videoserver.h"

///////////////////////////////////////////////////////////////////////

ClientConnectionHandler::ClientConnectionHandler(TS3VideoServer *server, QCorConnection *connection, QObject *parent) :
  QObject(parent),
  _server(server),
  _connection(connection)
{

}

ClientConnectionHandler::~ClientConnectionHandler()
{

}