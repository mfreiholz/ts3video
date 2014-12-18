#include "qcorframe.h"

QCorFrame::QCorFrame(QCorConnection *connection) :
  _connection(connection),
  _state(TransferingState),
  _type(RequestType)
{

}

QCorFrame::~QCorFrame()
{
}

QCorConnection* QCorFrame::connection() const
{
  return _connection;
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