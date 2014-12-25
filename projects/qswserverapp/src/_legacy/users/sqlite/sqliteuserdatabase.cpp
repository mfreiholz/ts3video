#include "_legacy/users/sqlite/sqliteuserdatabase.h"

#include "QtCore/QMutex"
#include "QtCore/QMutexLocker"
#include "QtCore/QDir"
#include "QtCore/QFileInfo"
#include "QtCore/QFile"
#include "QtCore/QStringList"

#include "QtSql/QSqlDatabase"
#include "QtSql/QSqlQuery"
#include "QtSql/QSqlError"

#include "humblelogging/api.h"

#include "shared/settings/settingsservice.h"

HUMBLE_LOGGER(HL, "userdatabase.sqllite");

///////////////////////////////////////////////////////////////////////////////
// SQLite User Object
///////////////////////////////////////////////////////////////////////////////

class UserImpl : public UserEntity
{
public:
	UserImpl()
	{}
	
	UserImpl(const QByteArray &fingerprint, const QByteArray &publicKey)
		: _fingerprint(fingerprint),
		  _publicKey(publicKey)
	{}

	QByteArray getFingerprint() const { return _fingerprint; }
	QByteArray getPublicKey() const { return _publicKey; }
	QString getName() const { return _name; }
	QString getEmailAddress() const { return _emailAddress; }
	QString getPhoneNumber() const { return _phoneNumber; }

public:
	QByteArray _fingerprint;
	QByteArray _publicKey;
	QString _name;
	QString _emailAddress;
	QString _phoneNumber;
};

///////////////////////////////////////////////////////////////////////////////
// Private Data Object
///////////////////////////////////////////////////////////////////////////////

BEGIN_PRIVATE_IMPL(SQLiteUserDatabase)
	mutable QMutex _m;
	SettingsRef _config;
	QString _dbFilePath;
	QSqlDatabase _db;

	void init() {}
	UserImpl* createFromQuery(QSqlQuery &query) const;
END_PRIVATE_IMPL(SQLiteUserDatabase)

///////////////////////////////////////////////////////////////////////////////
// SQLiteUserDatabase
///////////////////////////////////////////////////////////////////////////////

SQLiteUserDatabase::SQLiteUserDatabase()
	: UserDatabase(), INIT_PRIVATE(SQLiteUserDatabase)
{
	d->_config = SettingsService::instance().getUserSettings(); // TODO Change to system settings.
	d->_dbFilePath = d->_config->value("sqlite/database").toString();
}

SQLiteUserDatabase::~SQLiteUserDatabase()
{
	d->_db.close();
}

bool SQLiteUserDatabase::initialize()
{
	QString filePath = d->_dbFilePath;
	if (filePath.isEmpty()) {
		filePath = QDir::temp().filePath("users.db");
	}

	bool createTables = false;
	const QFileInfo sqliteDbInfo(filePath);
	if (!sqliteDbInfo.exists())
		createTables = true;

	d->_db = QSqlDatabase::addDatabase("QSQLITE", "users_connection");
	d->_db.setHostName("127.0.0.1");
	d->_db.setDatabaseName(filePath);
	d->_db.setUserName(QString());
	d->_db.setPassword(QString());

	bool errorOccured = false;
	if (!d->_db.open()) {
		HL_ERROR(HL, "Can not open database connection. Check your file permissions.");
		errorOccured = true;
	} else if (createTables) {
		QFile f(":/users.sqlite.sql");
		if (!f.open(QIODevice::ReadOnly)) {
			errorOccured = true;
		} else {
			QString sql = f.readAll();
			QStringList statements = sql.split(";", QString::SkipEmptyParts);
			foreach (const QString &stmt, statements) {
				QSqlQuery query(d->_db);
				if (!query.exec(stmt)) {
					HL_ERROR(HL, QString("SQL error (error=%1)").arg(query.lastError().text()).toStdString());
					d->_db.close();
					QFile::remove(sqliteDbInfo.filePath());
					return false;
				}
			}
		}
	}

	if (d->_db.lastError().isValid() && d->_db.lastError().type() != QSqlError::NoError)
		HL_ERROR(HL, QString("Last database error (error=%1)").arg(d->_db.lastError().text()).toStdString());
	if (!errorOccured)
		HL_INFO(HL, QString("Using SQLITE database (database=%1)").arg(filePath).toStdString());

	return !errorOccured;
}

quint64 SQLiteUserDatabase::getUserCount() const
{
	QMutexLocker l(&d->_m);
	QSqlQuery query(d->_db);
	query.prepare("SELECT COUNT(publickey_id) FROM publickeys");
	query.exec();
	if (query.next()) {
		return query.value(0).toULongLong();
	}
	return 0LL;
}

UserList SQLiteUserDatabase::getUsers(quint64 offset, quint64 len) const
{
	QMutexLocker l(&d->_m);
	UserList users;
	QSqlQuery query(d->_db);
	query.prepare("SELECT publickey_id,publickey,fingerprint,name,email,phone FROM publickeys LIMIT ?,?");
	query.addBindValue(offset);
	query.addBindValue(len);
	query.exec();
	while (query.next()) {
		auto user = d->createFromQuery(query);
		users.append(UserRef(user));
	}
	return users;
}

UserRef SQLiteUserDatabase::findUserByFingerprint(const QByteArray &fingerprint) const
{
	QMutexLocker l(&d->_m);
	QSqlQuery query(d->_db);
	query.prepare("SELECT publickey_id,publickey,fingerprint,name,email,phone FROM publickeys WHERE fingerprint = ?");
	query.addBindValue(QString::fromLatin1(fingerprint));
	query.exec();
	if (query.next()) {
		auto user = d->createFromQuery(query);
		return UserRef(user);
	}
	return UserRef();
}

bool SQLiteUserDatabase::addUser(const UserRef &user)
{
	QMutexLocker l(&d->_m);
	QSqlQuery query(d->_db);
	query.prepare("INSERT INTO `publickeys` (`publickey`, `fingerprint`) VALUES (?,?)");
	query.addBindValue(QString::fromLatin1(user->getPublicKey()));
	query.addBindValue(QString::fromLatin1(user->getFingerprint()));
	if (!query.exec()) {
		HL_ERROR(HL, QString("Can not insert new user into database (sql-error=%1)").arg(query.lastError().text()).toStdString());
		return false;
	}
	return true;
}

bool SQLiteUserDatabase::updateUser(const UserRef &user)
{
	QMutexLocker l(&d->_m);
	QSqlQuery query(d->_db);
	query.prepare("UPDATE publickeys "
		" SET name = ?, email = ?, phone = ? "
		" WHERE fingerprint = ? ");
	query.addBindValue(user->getName());
	query.addBindValue(user->getEmailAddress());
	query.addBindValue(user->getPhoneNumber());
	query.addBindValue(QString::fromLatin1(user->getFingerprint()));
	if (!query.exec()) {
		HL_ERROR(HL, QString("Can not update user in database (sql-error=%1)").arg(query.lastError().text()).toStdString());
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// SQLiteUserDatabase::Private
///////////////////////////////////////////////////////////////////////////////

UserImpl* SQLiteUserDatabase::Private::createFromQuery(QSqlQuery &query) const
{
	auto user = new UserImpl();
	user->_publicKey = query.value(1).toString().toLatin1();
	user->_fingerprint = query.value(2).toString().toLatin1();
	user->_name = query.value(3).toString();
	user->_emailAddress = query.value(4).toString();
	user->_phoneNumber = query.value(5).toString();
	return user;
}
