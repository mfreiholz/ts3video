#include "qcorrequest.h"

QCorRequest::QCorRequest(QCorConnection *connection, QObject *parent) :
  QObject(parent),
  _connection(connection)
{

}

QCorConnection* QCorRequest::connection() const
{
  return _connection;
}
