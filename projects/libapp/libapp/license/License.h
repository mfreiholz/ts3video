#pragma once

#include <QString>
#include <QDateTime>
#include <QList>

namespace lic {

class License
{
public:
	QString version;
	QString id;
	QDateTime createdOn;

	QDateTime expiresOn;
	bool expiresOnNever = false;

	QList<QString> validVersions;

	QString requiredHostId;
	QString requiredMacAddress;
	QString requiredSerial;

	QString customerId;
	QString customerName;
	QString customerEmail;
};

}
