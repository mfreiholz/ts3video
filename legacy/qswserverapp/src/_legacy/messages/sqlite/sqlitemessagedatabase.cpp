#include "_legacy/messages/sqlite/sqlitemessagedatabase.h"

#include "QtCore/QMutex"
#include "QtCore/QMutexLocker"
#include "QtCore/QDir"
#include "QtCore/QFileInfo"
#include "QtCore/QFile"
#include "QtCore/QStringList"
#include "QtCore/QDateTime"

#include "QtSql/QSqlDatabase"
#include "QtSql/QSqlQuery"
#include "QtSql/QSqlError"

#include "humblelogging/api.h"

#include "shared/settings/settingsservice.h"

HUMBLE_LOGGER(HL, "messagedatabase.sqllite");

///////////////////////////////////////////////////////////////////////////////
// SQLite Message Object
///////////////////////////////////////////////////////////////////////////////

class MessageImpl : public MessageEntity
{
public:
	MessageImpl()
	{}

	MessageImpl(quint64 id, const QByteArray &sender, const QByteArray &receiver, const QByteArray &message)
		: _id(id),
		  _sender(sender),
		  _receiver(receiver),
		  _message(message)
	{}

	quint64 getId() const { return _id; }
	QByteArray getSenderFingerprint() const { return _sender; }
	QByteArray getReceiverFingerprint() const { return _receiver; }
	QByteArray getMessage() const { return _message; }

public:
	quint64 _id;
	QByteArray _sender;
	QByteArray _receiver;
	QByteArray _message;
};

///////////////////////////////////////////////////////////////////////////////
// Private Data Object
///////////////////////////////////////////////////////////////////////////////

BEGIN_PRIVATE_IMPL(SQLiteMessageDatabase)
	mutable QMutex _m;
	SettingsRef _config;
	QString _dbFilePath;
	QSqlDatabase _db;

	void init() {}
END_PRIVATE_IMPL(SQLiteMessageDatabase)

///////////////////////////////////////////////////////////////////////////////
// SQLiteMessageDatabase
///////////////////////////////////////////////////////////////////////////////

SQLiteMessageDatabase::SQLiteMessageDatabase()
	: MessageDatabase(), INIT_PRIVATE(SQLiteMessageDatabase)
{
	d->_config = SettingsService::instance().getUserSettings(); // TODO Change to system settings.
	d->_dbFilePath = d->_config->value("sqlite/messagedatabase").toString();
}

SQLiteMessageDatabase::~SQLiteMessageDatabase()
{
	d->_db.close();
}

bool SQLiteMessageDatabase::initialize()
{
	QString filePath = d->_dbFilePath;
	if (filePath.isEmpty()) {
		filePath = QDir::temp().filePath("messages.db");
	}

	bool createTables = false;
	const QFileInfo sqliteDbInfo(filePath);
	if (!sqliteDbInfo.exists())
		createTables = true;

	d->_db = QSqlDatabase::addDatabase("QSQLITE", "messages_connection");
	d->_db.setHostName("127.0.0.1");
	d->_db.setDatabaseName(filePath);
	d->_db.setUserName(QString());
	d->_db.setPassword(QString());

	bool errorOccured = false;
	if (!d->_db.open()) {
		HL_ERROR(HL, "Can not open database connection. Check your file permissions.");
		errorOccured = true;
	} else if (createTables) {
		QFile f(":/messages.sqlite.sql");
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

MessageList SQLiteMessageDatabase::getUnreadMessages(const QByteArray &receiver) const
{
	QMutexLocker l(&d->_m);
	MessageList messages;
	QSqlQuery query(d->_db);
	query.prepare("SELECT message_id,sender,receiver,creationdate,fetcheddate,message "
		" FROM messages "
		" WHERE fetcheddate IS NULL AND receiver = ? "
		" ORDER BY creationdate ASC ");
	query.addBindValue(QString::fromLatin1(receiver));
	query.exec();
	while (query.next()) {
		auto message = new MessageImpl();
		message->_id = query.value(0).toULongLong();
		message->_sender = query.value(1).toString().toLatin1();
		message->_receiver = query.value(2).toString().toLatin1();
		message->_message = query.value(5).toByteArray();
		messages.append(MessageRef(message));
	}
	return messages;
}

quint64 SQLiteMessageDatabase::storeMessage(const QByteArray &sender, const QByteArray &receiver, const QByteArray &message)
{
	QMutexLocker l(&d->_m);
	QSqlQuery query(d->_db);
	query.prepare("INSERT INTO `messages` (`sender`,`receiver`,`creationdate`,`message`) VALUES (?,?,?,?)");
	query.addBindValue(QString::fromLatin1(sender));
	query.addBindValue(QString::fromLatin1(receiver));
	query.addBindValue(QDateTime::currentDateTimeUtc());
	query.addBindValue(message);
	if (!query.exec()) {
		HL_ERROR(HL, QString("Can not store message (sql-error=%1)").arg(query.lastError().text()).toStdString());
		return false;
	}
	return query.lastInsertId().toULongLong();
}

bool SQLiteMessageDatabase::markAsRead(quint64 messageId)
{
	QMutexLocker l(&d->_m);
	QSqlQuery query(d->_db);
	query.prepare("UPDATE messages SET fetcheddate = ? WHERE message_id = ?");
	query.addBindValue(QDateTime::currentDateTimeUtc());
	query.addBindValue(messageId);
	if (!query.exec()) {
		HL_ERROR(HL, QString("Can not mark message as read (sql-error=%1; message-id=%2)").arg(query.lastError().text()).arg(messageId).toStdString());
		return false;
	}
	return true;
}
