#include "clientapplogic.h"

ClientAppLogic::ClientAppLogic(QObject *parent) :
  QObject(parent)
{
  _ts3vc.connectToHost(QHostAddress("127.0.0.1"), 6000);
  connect(&_ts3vc, &TS3VideoClient::connected, this, &ClientAppLogic::onConnected);
  connect(&_ts3vc, &TS3VideoClient::disconnected, this, &ClientAppLogic::onDisconnected);
  connect(&_ts3vc, &TS3VideoClient::clientJoinedChannel, this, &ClientAppLogic::onClientJoinedChannel);
  connect(&_ts3vc, &TS3VideoClient::clientLeftChannel, this, &ClientAppLogic::onClientLeftChannel);
  connect(&_ts3vc, &TS3VideoClient::clientDisconnected, this, &ClientAppLogic::onClientDisconnected);
}

ClientAppLogic::~ClientAppLogic()
{

}

void ClientAppLogic::onConnected()
{
  // TODO Authenticate.
  // TODO Join channel.
}

void ClientAppLogic::onDisconnected()
{
  // TODO Exit application.
}

void ClientAppLogic::onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  // TODO Create new widget for client.
}

void ClientAppLogic::onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  // TODO Close widget of client.
}

void ClientAppLogic::onClientDisconnected(const ClientEntity &client)
{
  // TODO Close widget of client.
}
