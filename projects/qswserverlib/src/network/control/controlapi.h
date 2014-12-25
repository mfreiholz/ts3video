#ifndef CONTROLAPI_HEADER
#define CONTROLAPI_HEADER

#include <QtGlobal>
#include <QDataStream>
#include <QByteArray>
#include <QLatin1String>

/**
 * The minimum module request over the client connection protocol looks like this:
 *
 *   (4 byte) unsigned int -> Length of the following bytes.
 *   (x byte) char* -> Module identifier (latin-1 string)
 *   (4 byte) unsigned int -> Length of the following bytes.
 *   (x byte) char* -> Module's action identifier (latin-1 string)
 *
 * All other data is dynamicly handled by the module's action itself.
 */

namespace ControlProtocol {

struct ModuleActionRequest {
  static const int MIN_SIZE = sizeof(char) + sizeof(uint) + sizeof(char) + sizeof(uint);

  ModuleActionRequest() :
    module(0),
    moduleLength(0),
    action(0),
    actionLength(0)
  {
  }

  ModuleActionRequest(const ModuleActionRequest &other)
  {
    memcpy(this->module, other.module, other.moduleLength);
    this->moduleLength = other.moduleLength;
    memcpy(this->action, other.action, other.actionLength);
    this->actionLength = other.actionLength;
  }

  ~ModuleActionRequest()
  {
    delete[] module;
    delete[] action;
  }

  bool fromData(const QByteArray &data)
  {
    if (data.size() < MIN_SIZE) {
      return false;
    }
    QDataStream in(data);
    in.setByteOrder(QDataStream::BigEndian);
    in.readBytes(this->module, this->moduleLength);
    in.readBytes(this->action, this->actionLength);
    return true;
  }

  bool fromParams(const QLatin1String &moduleId, const QLatin1String &actionId)
  {
    this->module = new char[moduleId.size()];
    memcpy(this->module, moduleId.latin1(), moduleId.size());
    moduleLength = moduleId.size();

    this->action = new char[actionId.size()];
    memcpy(this->action, actionId.latin1(), actionId.size());
    moduleLength = actionId.size();
    return true;
  }

  void serialize(QDataStream &out)
  {
    out.writeBytes(this->module, this->moduleLength);
    out.writeBytes(this->action, this->actionLength);
  }

  char *module;
  uint moduleLength;

  char *action;
  uint actionLength;
};

} // End of namespace.
#endif