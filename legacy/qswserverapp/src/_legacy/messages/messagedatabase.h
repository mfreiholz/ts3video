#ifndef MESSAGEDATABASE_HEADER
#define MESSAGEDATABASE_HEADER

#include "QtCore/QList"
#include "QtCore/QSharedPointer"
class QByteArray;
class MessageEntity;

typedef QSharedPointer<MessageEntity> MessageRef;
typedef QList<MessageRef> MessageList;

/*!
	\class MessageDatabase
	
	<h2>Subclassing</h2>
	Subclassing is pretty straight forward...
	Note: Every subclass HAVE TO BE thread-safe!
*/
class MessageDatabase
{
public:
	virtual ~MessageDatabase() {};
	virtual bool initialize() = 0;

	virtual MessageList getUnreadMessages(const QByteArray &receiver) const = 0;

	virtual quint64 storeMessage(const QByteArray &sender, const QByteArray &receiver, const QByteArray &message) = 0;
	virtual bool markAsRead(quint64 messageId) = 0;
};

class MessageEntity
{
public:
	virtual quint64 getId() const = 0;
	virtual QByteArray getSenderFingerprint() const = 0;
	virtual QByteArray getReceiverFingerprint() const = 0;
	virtual QByteArray getMessage() const = 0;
};


#endif
