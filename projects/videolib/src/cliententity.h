#ifndef CLIENTENTITY_H
#define CLIENTENTITY_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>
#include <QHostAddress>

/*!
    This object always needs to be fully copyable.
*/
class ClientEntity
{
public:
	ClientEntity();
	ClientEntity(const ClientEntity& other);
	ClientEntity& operator=(const ClientEntity& other);
	~ClientEntity();
	
	void fromQJsonObject(const QJsonObject& obj);
	QJsonObject toQJsonObject() const;
	QString toString() const;

public:
	int id; ///< The client's ID assigned by server.
	QString name; ///< Visible name of the client.

	QHostAddress mediaAddress; ///< Do not serialize, as long as we don't support P2P streaming.
	quint16 mediaPort; ///< Do not serialize, as long as we don't support P2P streaming.

	// Video settings.
	bool videoEnabled; ///< Indicates whether the client has video enabled.
	int videoWidth;
	int videoHeight;
	int videoBitrate;

	// Audio settings.
	bool audioInputEnabled; ///< Indicates whether the client has audio input enabled (microphone).
};
Q_DECLARE_METATYPE(ClientEntity);
#endif