#include "mycorserver.h"
#include "qcorserver.h"
#include "qcorframe.h"


MyCorServer::MyCorServer(QObject *parent) :
  QObject(parent),
  _server(new QCorServer(this))
{
  _server->listen(QHostAddress::Any, 5005);
}

void MyCorServer::onNewFrame(QCorFrame *frame)
{
  QObject::connect(frame, SIGNAL(end()), frame, SLOT(deleteLater()));
}
