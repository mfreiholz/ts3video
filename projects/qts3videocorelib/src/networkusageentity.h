#ifndef NETWORKUSAGEENTITY_H
#define NETWORKUSAGEENTITY_H

#include <QMetaType>

class NetworkUsageEntity
{
public:
  NetworkUsageEntity();

  quint64 bytesRead;
  quint64 bytesWritten;

  double bandwidthRead;
  double bandwidthWrite;
};
Q_DECLARE_METATYPE(NetworkUsageEntity);
#endif