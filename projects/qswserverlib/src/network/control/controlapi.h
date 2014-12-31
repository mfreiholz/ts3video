#ifndef CONTROLAPI_HEADER
#define CONTROLAPI_HEADER

#include <QtGlobal>
#include <QDataStream>
#include <QByteArray>
#include <QLatin1String>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  uint16_t moduleLength;
  uint8_t *module;
  
  uint16_t actionLength;
  uint8_t *action;
  
  uint32_t bodyLength;
  uint8_t *body;

  static const size_t MINSIZE = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t);
} ma_request;

typedef struct
{
  uint32_t bodyLength;
  uint8_t *body;
} ma_response;

#ifdef __cplusplus
}
#endif

namespace ControlProtocol {

struct ModuleActionRequest {
  static const int MIN_SIZE = 8;

  ModuleActionRequest()
  {
  }

  ModuleActionRequest(const ModuleActionRequest &other)
  {
    this->module = other.module;
    this->action = other.action;
  }

  ~ModuleActionRequest()
  {
  }

  bool fromData(const QByteArray &data)
  {
    if (data.size() < MIN_SIZE) {
      return false;
    }
    QDataStream in(data);
    in.setByteOrder(QDataStream::BigEndian);
    
    quint32 len = 0;
    in >> len;
    char *buff = new char[len];
    in.readRawData(buff, len);
    this->module = QString::fromLatin1(buff, len);
    delete[] buff;

    len = 0;
    in >> len;
    buff = new char[len];
    in.readRawData(buff, len);
    this->action = QString::fromLatin1(buff, len);
    delete[] buff;

    return true;
  }

  void serialize(QDataStream &out)
  {
    out << (quint32) this->module.length();
    out.writeRawData(this->module.toLatin1().constData(), this->module.length());

    out << (quint32) this->action.length();
    out.writeRawData(this->module.toLatin1().constData(), this->action.length());
  }

  QString module;
  QString action;
};

} // End of namespace.
#endif