#include "license.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QRegExp>
#include <QXmlStreamReader>

#include "ts3video.h"

namespace lic
{

// LicenseXmlReader ///////////////////////////////////////////////////////////

std::shared_ptr<License>
LicenseXmlReader::loadFromFile(const QString& filePath) const
{
	std::shared_ptr<License> lic;
	QFile f(filePath);
	if (!f.open(QIODevice::ReadOnly))
	{
		return lic;
	}

	lic = std::make_shared<License>();

	QXmlStreamReader xml(&f);
	while (!xml.atEnd())
	{
		auto tt = xml.readNext();
		if (tt == QXmlStreamReader::StartElement)
		{
			if (xml.name() == "id")
			{
				xml.readNext();
				lic->id = xml.text().toString();
			}
			else if (xml.name() == "created-on")
			{
				xml.readNext();
				lic->createdOn = QDateTime::fromString(xml.text().toString(), "yyyy-MM-ddTHH:mm:ss");
				lic->createdOn.setTimeSpec(Qt::UTC);
			}
			else if (xml.name() == "expires-on")
			{
				xml.readNext();
				lic->expiresOn = QDateTime::fromString(xml.text().toString(), "yyyy-MM-ddTHH:mm:ss");
				lic->expiresOn.setTimeSpec(Qt::UTC);
			}
		}
	}
	f.close();

	if (xml.error() != QXmlStreamReader::NoError)
	{
		lic.reset();
		return lic;
	}
	return lic;
}

// LicenseManager

// Checks whether the given "version" is >= than the current software version
static bool isVersionValid(const QString& version)
{
	bool isValid = true;

	QString softwareVersion(IFVS_SOFTWARE_VERSION);
	auto softwareVersionParts = softwareVersion.split(".", QString::SkipEmptyParts);

	auto l = version.split(".", QString::SkipEmptyParts);
	for (auto i = 0; i < softwareVersionParts.size() && i < l.count(); ++i)
	{
		if (l[i].compare("*") == 0)
		{
			continue; // Match!
		}
		auto sofNum = softwareVersionParts[i].toInt();
		auto licNum = l[i].toInt();
		if (licNum < sofNum)
		{
			isValid = false;
			break;
		}
	}

	return isValid;
}

bool
LicenseManager::isValid(const License& lic)
{
	if (lic.expiresOn.isValid())
	{
		auto now = QDateTime::currentDateTimeUtc();
		if (lic.expiresOn < now)
		{
			return false;
		}
	}

	bool validVersion = false;
	foreach (const QString& version, lic.validVersions)
	{
		if (isVersionValid(version))
		{
			validVersion = true;
			break;
		}
	}

	return true;
}

}