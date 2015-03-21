#ifndef SQLITEMESSAGEDATABASE_HEADER
#define SQLITEMESSAGEDATABASE_HEADER

#include "shared/api/apidef.h"

#include "_legacy/messages/messagedatabase.h"

class SQLiteMessageDatabase : public MessageDatabase
{
	DECLARE_PRIVATE(SQLiteMessageDatabase)
	DISABLE_COPY(SQLiteMessageDatabase)
public:
	SQLiteMessageDatabase();
	virtual ~SQLiteMessageDatabase();
	virtual bool initialize();
	virtual MessageList getUnreadMessages(const QByteArray &receiver) const;
	virtual quint64 storeMessage(const QByteArray &sender, const QByteArray &receiver, const QByteArray &message);
	virtual bool markAsRead(quint64 messageId);
};

#endif