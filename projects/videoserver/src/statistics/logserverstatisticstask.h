#ifndef LOGSERVERSTAISTICSTASK_H
#define LOGSERVERSTAISTICSTASK_H

#include <QRunnable>

#include "videolib/networkusageentity.h"

class LogVirtualServerStatisticsTask : public QRunnable
{
public:
	LogVirtualServerStatisticsTask(const NetworkUsageEntity& nue);
	virtual void run() override;

private:
	NetworkUsageEntity _networkUsage;
};

#endif