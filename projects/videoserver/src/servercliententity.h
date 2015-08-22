#ifndef SERVERCLIENTENTITY_H
#define SERVERCLIENTENTITY_H

#include "cliententity.h"
#include <QSet>
#include <QHash>
#include <QString>
#include <QVariant>

class ServerClientEntity : public ClientEntity
{
public:
  ServerClientEntity();
  ServerClientEntity(const ServerClientEntity& other);

public:
  QSet<int> remoteVideoExcludes; ///< "This" client, doesn't want to receive the video of clients in this list.
  QHash<QString, QVariant> customData; ///< Holds custom data for this client on server side (e.g.: "ts3.cldbid")
};

#endif