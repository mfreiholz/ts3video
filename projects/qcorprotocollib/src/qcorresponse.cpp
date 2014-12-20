#include "qcorresponse.h"

QCorResponse::QCorResponse(QObject *parent) :
  QObject(parent)
{

}

QCorFrameRefPtr QCorResponse::frame() const
{
  return _frame;
}