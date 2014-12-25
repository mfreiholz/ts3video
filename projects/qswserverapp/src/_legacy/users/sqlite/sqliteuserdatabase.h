#ifndef SQLITEUSERDATABASE_HEADER
#define SQLITEUSERDATABASE_HEADER

#include "shared/api/apidef.h"

#include "_legacy/users/userdatabase.h"

class SQLiteUserDatabase : public UserDatabase
{
	DECLARE_PRIVATE(SQLiteUserDatabase)
	DISABLE_COPY(SQLiteUserDatabase)
public:
	SQLiteUserDatabase();
	virtual ~SQLiteUserDatabase();
	virtual bool initialize();
	virtual quint64 getUserCount() const;
	virtual UserList getUsers(quint64 offset, quint64 len) const;
	virtual UserRef findUserByFingerprint(const QByteArray &fingerprint) const;
	virtual bool addUser(const UserRef &user);
	virtual bool updateUser(const UserRef &user);
};

#endif
