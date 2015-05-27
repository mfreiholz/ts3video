#include "coremodule.h"

///////////////////////////////////////////////////////////////////////

class Task
{
public:
  AbstractModuleBase::ActionRequest req;
  virtual void run() = 0;
};

class AuthenticationTask : public Task
{
public:
  void run()
  {

  }
};

///////////////////////////////////////////////////////////////////////

class CoreModulePrivate
{
public:
  QHash<QString, Task*> tasks;
};

CoreModule::CoreModule()
{

}

CoreModule::~CoreModule()
{
  qDeleteAll(d->tasks);
}

QSet<QString> CoreModule::actions() const
{
  return d->tasks.keys().toSet();
}

int CoreModule::processActionRequest(const AbstractModuleBase::ActionRequest &req)
{
  auto task = d->tasks.value(req.action);
  if (!task) {
    throw new ActionRequestException(req);
  }
  task->req = req;
  task->run();
}
