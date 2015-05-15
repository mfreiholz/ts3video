#ifndef VIRTUALSERVER_P_H
#define VIRTUALSERVER_P_H

#include <QObject>
#include <QHash>
#include "virtualserver.h"
class RequestActionHandlerI;

class VirtualServerPrivate : public QObject
{
  Q_OBJECT

public:
  VirtualServer *owner;
  QHash<QString, RequestActionHandlerI*> handlers;

public:
  VirtualServerPrivate(VirtualServer *o) : owner(o) {}
};

#endif