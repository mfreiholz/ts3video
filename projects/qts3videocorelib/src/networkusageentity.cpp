#include "networkusageentity.h"

#include <QVariant>

///////////////////////////////////////////////////////////////////////

NetworkUsageEntity::NetworkUsageEntity() :
  bytesRead(0),
  bytesWritten(0),
  bandwidthRead(0.0),
  bandwidthWrite(0.0)
{
}

void NetworkUsageEntity::fromQJsonObject(const QJsonObject &obj)
{
  this->bytesRead = obj["bytesread"].toVariant().toUInt();
  this->bytesWritten = obj["byteswritten"].toVariant().toUInt();
  this->bandwidthRead = obj["bandwidthread"].toDouble();
  this->bandwidthWrite = obj["bandwidthwrite"].toDouble();
}

QJsonObject NetworkUsageEntity::toQJsonObject() const
{
  QJsonObject obj;
  obj["bytesread"] = (qint64)this->bytesRead;
  obj["byteswritten"] = (qint64)this->bytesWritten;
  obj["bandwidthread"] = this->bandwidthRead;
  obj["bandwidthwrite"] = this->bandwidthWrite;
  return obj;
}

QString NetworkUsageEntity::toString() const
{
  QStringList sl;
  sl << QString::number(bytesRead) << QString::number(bytesWritten) << QString::number(bandwidthRead) << QString::number(bandwidthWrite);
  return sl.join("#");
}

///////////////////////////////////////////////////////////////////////

NetworkUsageEntityHelper::NetworkUsageEntityHelper(NetworkUsageEntity &networkUsage) :
  _networkUsage(networkUsage),
  _bandwidthReadTemp(0),
  _bandwidthWrittenTemp(0)
{
  _bandwidthCalcTime.start();
}

void NetworkUsageEntityHelper::recalculate()
{
  auto elapsedms = _bandwidthCalcTime.elapsed();
  _bandwidthCalcTime.restart();
  // Calculate READ transfer rate.
  if (elapsedms > 0) {
    auto diff = _networkUsage.bytesRead - _bandwidthReadTemp;
    if (diff > 0) {
      _networkUsage.bandwidthRead = ((double)diff / elapsedms) * 1000;
    }
    else {
      _networkUsage.bandwidthRead = 0.0;
    }
    _bandwidthReadTemp = _networkUsage.bytesRead;
  }
  // Calculate WRITE transfer rate.
  if (elapsedms > 0) {
    auto diff = _networkUsage.bytesWritten - _bandwidthWrittenTemp;
    if (diff > 0) {
      _networkUsage.bandwidthWrite = ((double)diff / elapsedms) * 1000;
    }
    else {
      _networkUsage.bandwidthWrite = 0.0;
    }
    _bandwidthWrittenTemp = _networkUsage.bytesWritten;
  }
}