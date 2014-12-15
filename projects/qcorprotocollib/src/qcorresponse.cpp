#include "qcorresponse.h"

QCorResponse::QCorResponse(QObject *parent /* = 0 */) :
  QIODevice(parent)
{

}

qint64 QCorResponse::readData(char *data, qint64 maxSize)
{
  return 0;
}

qint64 QCorResponse::writeData(const char *data, qint64 maxSize)
{
  return 0;
}