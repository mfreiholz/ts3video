#include "servercliententity.h"

ServerClientEntity::ServerClientEntity() :
	ClientEntity()
{
}

ServerClientEntity::ServerClientEntity(const ServerClientEntity& other) :
	ClientEntity(other)
{
}