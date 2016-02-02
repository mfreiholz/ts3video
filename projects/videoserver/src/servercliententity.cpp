#include "servercliententity.h"

ServerClientEntity::ServerClientEntity() :
	ClientEntity(),
	authenticated(false),
	admin(false),
	remoteVideoExcludes(),
	remoteVideoIncludes(),
	visibilityLevel(VL_Default),
	visibilityLevelAllowed(VL_Default)
{
}

ServerClientEntity::ServerClientEntity(const ServerClientEntity& other) :
	ClientEntity(other)
{
	this->authenticated = other.authenticated;
	this->admin = other.admin;
	this->remoteVideoExcludes = other.remoteVideoExcludes;
	this->remoteVideoIncludes = other.remoteVideoIncludes;
	this->visibilityLevel = other.visibilityLevel;
	this->visibilityLevelAllowed = other.visibilityLevelAllowed;
}

ServerClientEntity& ServerClientEntity::operator=(const ServerClientEntity& other)
{
	ClientEntity::operator=(other);
	this->authenticated = other.authenticated;
	this->admin = other.admin;
	this->remoteVideoExcludes = other.remoteVideoExcludes;
	this->remoteVideoIncludes = other.remoteVideoIncludes;
	this->visibilityLevel = other.visibilityLevel;
	this->visibilityLevelAllowed = other.visibilityLevelAllowed;
	return *this;
}

bool ServerClientEntity::isAllowedToSee(const ServerClientEntity& sce) const
{
	return (visibilityLevelAllowed >= sce.visibilityLevel);
}