#include "qcorresponse.h"

QCorResponse::QCorResponse(QCorConnection *connection, QCorRequest *request, QObject *parent) :
  QObject(parent),
  _connection(connection),
  _request(request)
{

}