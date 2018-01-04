#include "LicenseManager.h"
#include "License.h"

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QNetworkInterface>

#include "../ts3video.h"

namespace lic {

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

}
