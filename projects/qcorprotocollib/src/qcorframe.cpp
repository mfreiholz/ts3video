#include "qcorframe.h"

QCorFrame::QCorFrame(QCorConnection *connection, QObject *parent) :
  QObject(parent),
  _connection(connection),
  _state(TransferingState),
  _type(RequestType)
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

void QCorFrame::setState(State s)
{
  _state = s;
}

void QCorFrame::setType(Type t)
{
  _type = t;
}
