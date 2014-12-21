#include "qcorreply.h"

QCorReply::QCorReply(QObject *parent) :
  QObject(parent)
{

}

QCorFrameRefPtr QCorReply::frame() const
{
  return _frame;
}