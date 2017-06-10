#include "license.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QRegExp>
#include <QXmlStreamReader>
#include <QNetworkInterface>

#include "ts3video.h"

namespace lic {

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
			if (xml.name() == "license")
			{
				lic->version = xml.attributes().value("version").toString();
			}
			else if (xml.name() == "id")
			{
				xml.readNext();
				lic->id = xml.text().toString();
			}
			else if (xml.name() == "created-on")
			{
				xml.readNext();
				lic->createdOn = QDateTime::fromString(xml.text().toString(),
													   "yyyy-MM-ddTHH:mm:ss");
				lic->createdOn.setTimeSpec(Qt::UTC);
			}
			else if (xml.name() == "expires-on")
			{
				xml.readNext();
				auto s = xml.text().toString();
				if (s.compare("*") == 0)
				{
					lic->expiresOnNever = true;
				}
				else
				{
					lic->expiresOn = QDateTime::fromString(s, "yyyy-MM-ddTHH:mm:ss");
					lic->expiresOn.setTimeSpec(Qt::UTC);
				}
			}
			else if (xml.name() == "valid-verions")
			{
				while (!xml.atEnd())
				{
					tt = xml.readNext();
					if (tt == QXmlStreamReader::EndElement && xml.name() == "valid-versions")
						break;
					else if (tt == QXmlStreamReader::StartElement && xml.name() == "version")
					{
						xml.readNext();
						lic->validVersions.append(xml.text().toString());
					}
				}
			}
			else if (xml.name() == "requirements")
			{
				tt = xml.readNext();
				if (tt == QXmlStreamReader::EndElement && xml.name() == "requirements")
					break;
				else if (tt == QXmlStreamReader::StartElement && xml.name() == "hostid")
				{
					xml.readNext();
					lic->requiredHostId = xml.text().toString();
				}
				else if (tt == QXmlStreamReader::StartElement && xml.name() == "mac")
				{
					xml.readNext();
					lic->requiredMacAddress = xml.text().toString();
				}
				else if (tt == QXmlStreamReader::StartElement && xml.name() == "serial")
				{
					xml.readNext();
					lic->requiredSerial = xml.text().toString();
				}
			}
			else if (xml.name() == "customer")
			{
				tt = xml.readNext();
				if (tt == QXmlStreamReader::EndElement && xml.name() == "customer")
					break;
				else if (tt == QXmlStreamReader::StartElement && xml.name() == "id")
				{
					xml.readNext();
					lic->customerId = xml.text().toString();
				}
				else if (tt == QXmlStreamReader::StartElement && xml.name() == "name")
				{
					xml.readNext();
					lic->customerName = xml.text().toString();
				}
				else if (tt == QXmlStreamReader::StartElement && xml.name() == "email")
				{
					xml.readNext();
					lic->customerEmail = xml.text().toString();
				}
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
	// Check expiration date.
	if (!lic.expiresOn.isValid() && !lic.expiresOnNever)
	{
		return false;
	}
	else if (lic.expiresOn.isValid() && !lic.expiresOnNever)
	{
		auto now = QDateTime::currentDateTimeUtc();
		if (lic.expiresOn < now)
		{
			return false;
		}
	}

	// Check version.
	bool validVersion = false;
	foreach (const QString& version, lic.validVersions)
	{
		if (isVersionValid(version))
		{
			validVersion = true;
			break;
		}
	}

	// Check host-id.
	// TODO...

	// Check MAC address.
	bool validMacAddress = true;
	if (!lic.requiredMacAddress.isEmpty())
	{
		validMacAddress = false;
		auto interfaces = QNetworkInterface::allInterfaces();
		for (const auto& iface : interfaces)
		{
			auto mac = iface.hardwareAddress();
			if (mac.compare(lic.requiredMacAddress, Qt::CaseInsensitive) == 0)
			{
				validMacAddress = true;
				break;
			}
		}
	}

	// Check serial.
	bool validSerial = true;
	if (!lic.requiredSerial.isEmpty())
	{
		validSerial = false;
		if (lic.requiredSerial.compare("READ SERIAL FROM FILE?") == 0)
			validSerial = true;
	}

	return validVersion && validMacAddress && validSerial;
}

QFuture<std::shared_ptr<License> >
LicenseManager::loadLicenseFromServer()
{
	QFuture<std::shared_ptr<License> > f;
	return f;
}

}