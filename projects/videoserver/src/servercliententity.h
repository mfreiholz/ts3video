#ifndef SERVERCLIENTENTITY_H
#define SERVERCLIENTENTITY_H

#include "cliententity.h"
#include <QSet>

class ServerClientEntity : public ClientEntity
{
public:
	ServerClientEntity();
	ServerClientEntity(const ServerClientEntity& other);

public:
	QSet<int> remoteVideoExcludes; ///< "This" client, doesn't want to receive the video of clients in this list.
};

#endif