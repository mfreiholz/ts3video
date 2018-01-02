#pragma once
#include <inttypes.h>
#include <QString>

namespace account {

class User
{
public:
	uint64_t id;
	QString email;
	QString firstname;
	QString lastname;
};

}
