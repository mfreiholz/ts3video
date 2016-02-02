#ifndef SERVERCLIENTENTITY_H
#define SERVERCLIENTENTITY_H

#include <QSet>

#include "baselib/defines.h"

#include "cliententity.h"

class ServerClientEntity : public ClientEntity
{
public:
	enum VisibilityLevel
	{
		VL_Default = 0, ///< Visible to everyone
		VL_Admin = 100, ///< Visible to administrators
		VL_NotVisible = 999 ///< This client is not visible to anyone
	};

	ServerClientEntity();
	ServerClientEntity(const ServerClientEntity& other);
	ServerClientEntity& operator=(const ServerClientEntity& other);

	bool isAllowedToSee(const ServerClientEntity& sce) const;

public:
	bool authenticated;
	bool admin;
	QSet<ocs::clientid_t> remoteVideoExcludes; ///< "This" client, doesn't want to receive the video of clients in this list.
	QSet<ocs::clientid_t> remoteVideoIncludes; // "This" client wants to receive video of these clients (It doesn't matter, whether the clients are in the same conference or not!)

	VisibilityLevel visibilityLevel; ///< The VL of the client itself.
	VisibilityLevel visibilityLevelAllowed; ///< The maximum VL this client is allowed to see.
};

#endif