#include "servercliententity.h"

ServerClientEntity::ServerClientEntity() :
	ClientEntity(), authenticated(false), admin(false)
{
}

ServerClientEntity::ServerClientEntity(const ServerClientEntity& other) :
	ClientEntity(other)
{
	this->authenticated = other.authenticated;
	this->admin = other.admin;
	this->remoteVideoExcludes = other.remoteVideoExcludes;
}

ServerClientEntity& ServerClientEntity::operator=(const ServerClientEntity& other)
{
	ClientEntity::operator=(other);
	this->authenticated = other.authenticated;
	this->admin = other.admin;
	this->remoteVideoExcludes = other.remoteVideoExcludes;
	return *this;
}