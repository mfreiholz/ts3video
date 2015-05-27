#ifndef COREMODULE_H
#define COREMODULE_H

#include <QScopedPointer>
#include "abstractmodulebase.h"

class CoreModulePrivate;
class CoreModule : public AbstractModuleBase
{
  QScopedPointer<CoreModulePrivate> d;
public:
  CoreModule();
  ~CoreModule();
  QSet<QString> actions() const;
  int processActionRequest(const ActionRequest &req);
};

#endif
