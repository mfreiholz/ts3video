#include "qcorframe.h"

QCorFrame::QCorFrame() :
  _state(TransferingState),
  _type(RequestType)
{

}

QCorFrame::~QCorFrame()
{
}

QCorFrame::State QCorFrame::state() const
{
  return _state;
}

QCorFrame::Type QCorFrame::type() const
{
  return _type;
}

QByteArray QCorFrame::data() const
{
  return _data;
}