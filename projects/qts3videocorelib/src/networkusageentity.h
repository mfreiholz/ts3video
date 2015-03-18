#ifndef NETWORKUSAGEENTITY_H
#define NETWORKUSAGEENTITY_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>
#include <QTime>

class NetworkUsageEntity
{
public:
  NetworkUsageEntity();
  void fromQJsonObject(const QJsonObject &obj);
  QJsonObject toQJsonObject() const;
  QString toString() const;

public:
  quint64 bytesRead;
  quint64 bytesWritten;

  double bandwidthRead;
  double bandwidthWrite;
};
Q_DECLARE_METATYPE(NetworkUsageEntity);

/*! Helper class for calculating the bandwidth usage.
 */
class NetworkUsageEntityHelper
{
public:
  NetworkUsageEntityHelper(NetworkUsageEntity &networkUsage);
  void recalculate();

private:
  NetworkUsageEntity &_networkUsage;
  QTime _bandwidthCalcTime;
  quint64 _bandwidthReadTemp;
  quint64 _bandwidthWrittenTemp;
};

#endif