#ifndef SERVERCLIENTENTITY_H
#define SERVERCLIENTENTITY_H

#include "cliententity.h"
#include <QSet>

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
	QSet<int> remoteVideoExcludes; ///< "This" client, doesn't want to receive the video of clients in this list.

	VisibilityLevel visibilityLevel; ///< The VL of the client itself.
	VisibilityLevel visibilityLevelAllowed; ///< The maximum VL this client is allowed to see.
};

#endif