#ifndef VIRTUALSERVER_P_H
#define VIRTUALSERVER_P_H

#include <QObject>
#include <QHash>
#include <QSharedPointer>
#include "virtualserver.h"
#include "action/actionbase.h"

class VirtualServerPrivate : public QObject
{
  Q_OBJECT
public:
  VirtualServer *owner;
  QHash<QString, ActionPtr> actions;

  VirtualServerPrivate(VirtualServer *o) :
    owner(o)
  {}

  void registerAction(const ActionPtr &action)
  {
    if (actions.contains(action->name())) {
      return;
    }
    actions.insert(action->name(), action);
  }

};

#endif
