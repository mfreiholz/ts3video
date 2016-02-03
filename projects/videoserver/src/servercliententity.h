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
		VL_Default = 0,          // Visible to everyone
		VL_Admin = 100,          // Visible to administrators
		VL_NotVisible = 999      // This client is not visible to anyone
	};

	ServerClientEntity();
	ServerClientEntity(const ServerClientEntity& other);
	ServerClientEntity& operator=(const ServerClientEntity& other);

	bool isAllowedToSee(const ServerClientEntity& sce) const;

public:
	// Indicates whether the client is authenticated
	bool authenticated;

	// Indicates whether the client is authorized as Administrator
	bool admin;

	// The VL of the client itself.
	VisibilityLevel visibilityLevel;

	// The maximum VL this client is allowed to see.
	VisibilityLevel visibilityLevelAllowed;
};

#endif