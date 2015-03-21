#ifndef USERDATABASE_HEADER
#define USERDATABASE_HEADER

#include "QtCore/QList"
#include "QtCore/QSharedPointer"
#include "shared/api/apidef.h"
class QByteArray;
class UserEntity;

typedef QSharedPointer<UserEntity> UserRef;
typedef QList<UserRef> UserList;

/*!
	<h2>Subclassing</h2>
	Subclassing is pretty straight forward...
	Note: Every subclass HAVE TO BE thread-safe!
*/
class UserDatabase
{
public:
	virtual ~UserDatabase() {};
	virtual bool initialize() = 0;

	virtual quint64 getUserCount() const = 0;
	virtual UserList getUsers(quint64 offset, quint64 len) const = 0;
	virtual UserRef findUserByFingerprint(const QByteArray &fingerprint) const = 0;

	virtual bool addUser(const UserRef &user) = 0;
	virtual bool updateUser(const UserRef &user) = 0;
};

/*!
	Basic API of a single user.
	It provides access to most used information.
*/
class UserEntity
{
public:
	virtual QByteArray getFingerprint() const = 0;
	virtual QByteArray getPublicKey() const = 0;
	virtual QString getName() const = 0;
	virtual QString getEmailAddress() const = 0;
	virtual QString getPhoneNumber() const = 0;
};

/*!
	Helper implementation of UserEntity.
	Can be used to create/update users.
*/
class UserEntityImpl
	: public UserEntity
{
public:
	virtual QByteArray getFingerprint() const { return fingerprint; }
	virtual QByteArray getPublicKey() const { return publicKey; }
	virtual QString getName() const { return name; }
	virtual QString getEmailAddress() const { return email; }
	virtual QString getPhoneNumber() const { return phone; }

public:
	QByteArray fingerprint;
	QByteArray publicKey;
	QString name;
	QString email;
	QString phone;
};

#endif
