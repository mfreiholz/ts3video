#include "qcorreply.h"

QCorReply::QCorReply(QObject *parent) :
  QObject(parent)
{

}

QCorFrameRefPtr QCorReply::frame() const
{
  return _frame;
}

void QCorReply::autoDelete(QCorReply* reply)
{
  QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
}
