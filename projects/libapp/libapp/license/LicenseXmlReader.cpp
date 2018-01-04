#include "LicenseXmlReader.h"
#include "License.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QDateTime>

namespace lic {

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

}
