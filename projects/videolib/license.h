#pragma once

#include <memory>

#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>

namespace lic
{


class License
{
public:
	QString id;
	QDateTime createdOn;
	QDateTime expiresOn;
	QList<QString> validVersions;
};


class LicenseXmlReader
{
public:
	std::shared_ptr<License> loadFromFile(const QString& filePath) const;
};


class LicenseManager
{
public:
	static bool isValid(const License& lic);
};

}