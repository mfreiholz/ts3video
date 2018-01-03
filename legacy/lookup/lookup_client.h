#pragma once

#include <memory>
#include <QObject>

class VersionEntity
{
public:
	QString version;
	QString homepageUrl;
	QString message;
	QString releasedOn;

	bool fromJson(const QJsonObject& json);
};

/*
	Class to interact with the central conference-/server-lookup
	service (api.mfreiholz.de)
*/
class LookupServiceClient :
	public QObject
{
	Q_OBJECT

public:
	LookupServiceClient(QObject* parent);
	virtual ~LookupServiceClient();

	/*
		retrieves a list of available updates from remote service.
		\throws ::ocs::CoreException
	*/
	QList<VersionEntity> checkForUpdates();

private:
	class Private;
	std::unique_ptr<Private> d;
};