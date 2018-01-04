#ifndef VIRTUALSERVEROPTIONS_H
#define VIRTUALSERVEROPTIONS_H

#include <limits>

#include <QtGlobal>
#include <QList>
#include <QString>
#include <QSize>
#include <QHostAddress>

#include "libapp/ts3video.h"

class VirtualServerOptions
{
public:
	// The address and port on which the server listens for new connections.
	QHostAddress address = QHostAddress::Any;
	quint16 port = IFVS_SERVER_CONNECTION_PORT;

	// The address and port of server's status and control WebSocket.
	QHostAddress wsStatusAddress = QHostAddress::Any;
	quint16 wsStatusPort = IFVS_SERVER_WSSTATUS_PORT;

	// The maximum number of parallel client connections.
	int connectionLimit = std::numeric_limits<int>::max();

	// The maximum bandwidth the server may use.
	// New connections will be blocked, if the server's bandwidth
	// usage reaches this value.
	unsigned long long bandwidthReadLimit = std::numeric_limits<unsigned long long>::max();
	unsigned long long bandwidthWriteLimit = std::numeric_limits<unsigned long long>::max();

	// List of valid channel IDs users are allowed to join.
	// Leave empty for no restrictions on channel-ids.
	//QList<int> validConferenceIds;

	// Basic server password.
	QString password;

	// Administrator password.
	QString adminPassword;

	// Maximum supported resolution and quality.
	QSize maximumResolution = QSize(1920, 1080);
	int maximumBitrate = 1024;

	// TeamSpeak 3 Server Bridge.
	bool ts3Enabled = false;
	QHostAddress ts3Address = QHostAddress::LocalHost;
	quint16 ts3Port = 10011;
	QString ts3LoginName;
	QString ts3LoginPassword;
	QString ts3Nickname;
	quint16 ts3VirtualServerPort = 9987;
	QList<quint64> ts3AllowedServerGroups;
};

#endif