#ifndef VIRTUALSERVER_P_H
#define VIRTUALSERVER_P_H

#include <QObject>
#include <QHash>
#include <QSharedPointer>
#include "virtualserver.h"

class VirtualServerPrivate : public QObject
{
  Q_OBJECT

public:
  VirtualServer *owner;

  VirtualServerPrivate(VirtualServer *o) :
    owner(o)
  {}

};

#endif
