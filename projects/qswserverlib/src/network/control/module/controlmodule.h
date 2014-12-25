#ifndef CONTROLMODULE_HEADER
#define CONTROLMODULE_HEADER

#include <QByteArray>
#include <QString>

class ControlModule
{
public:
  class Action
  {
  public:
    QString action;
    QByteArray data;
  };

  class Result
  {
  public:
    QString errorMessage;
    QByteArray data;
  };

  ControlModule(const QString &name);
  virtual ~ControlModule();
  virtual bool initialize();
  virtual QString getName() const;
  virtual Result handleAction(const Action &action) = 0;

private:
  QString _name;
};

class DemoControlModule :
  public ControlModule
{
public:
  DemoControlModule();
  virtual Result handleAction(const Action &action);
};

#endif