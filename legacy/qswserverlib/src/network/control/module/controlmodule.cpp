#include "controlmodule.h"

///////////////////////////////////////////////////////////////////////
// Base client module
///////////////////////////////////////////////////////////////////////

ControlModule::ControlModule(const QString &name) :
  _name(name)
{
}

ControlModule::~ControlModule()
{
}

bool ControlModule::initialize()
{
  return true;
}

QString ControlModule::getName() const
{
  return _name;
}

///////////////////////////////////////////////////////////////////////
// Demo Module
///////////////////////////////////////////////////////////////////////

DemoControlModule::DemoControlModule() :
  ControlModule("demo")
{
}

ControlModule::Result DemoControlModule::handleAction(const ControlModule::Action &action)
{
  ControlModule::Result r;
  r.data = QString("DEMO CONTENT").toUtf8();
  return r;
}