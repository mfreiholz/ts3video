#ifndef ELWS_H
#define ELWS_H

#include <QString>
#include <QVariant>
#include <QHostAddress>
class QRect;
class QPoint;

class ELWS
{
public:
	static bool hasArgsValue(const QString& key);
	static QVariant getArgsValue(const QString& key, const QVariant& defaultValue = QVariant());

	static QString getUserName();

	static bool registerURISchemeHandler(const QString& scheme, const QString& title, const QString& modulePath, const QString& moduleArgs);
	static bool unregisterURISchemeHandler(const QString& scheme);

	static void calcScaledAndCenterizedImageRect(const QRect& surfaceRect, QRect& imageRect, QPoint& offset);

	static QString humanReadableSize(quint64 bytes);
	static QString humanReadableBandwidth(quint64 bytesPerSecond);

	/*!
		\note A version MUST be the combination of major and minor: X.X
		\param version Single version to check against, e.g.: 1.0
		\param supportedVersions Comma separated list of valid versions, e.g.: 1.0,2.0,3.0
	*/
	static bool isVersionSupported(const QString& clientVersion, const QString& serverVersion, const QString& clientSupportedServerVersions, const QString& serverSupportedClientVersions);

	static QHostAddress getQHostAddressFromString(const QString& s);

	/*
		resolves a DNS
	*/
	static QHostAddress resolveDns(const QString& dns);
};

#endif
