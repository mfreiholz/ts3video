#ifndef CLIENTAPPLOGIC_H
#define CLIENTAPPLOGIC_H

#include <QObject>
#include <QHash>

#include "ts3videoclient.h"

class QWidget;

class ClientAppLogic : public QObject
{
  Q_OBJECT

public:
  ClientAppLogic(QObject *parent);
  ~ClientAppLogic();

private slots:
  void onConnected();
  void onDisconnected();
  void onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientDisconnected(const ClientEntity &client);

protected:
  QWidget* createClientWidget(const ClientEntity &client);

private:
  TS3VideoClient _ts3vc;

  // Client widgets.
  QHash<int, QWidget*> _clientWidgets;
};

#endif