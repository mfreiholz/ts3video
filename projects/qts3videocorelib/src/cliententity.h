#ifndef CLIENTENTITY_H
#define CLIENTENTITY_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>

class ClientEntity
{
public:
  ClientEntity();
  ClientEntity(const ClientEntity &other);
  ~ClientEntity();
  void fromQJsonObject(const QJsonObject &obj);
  QJsonObject toQJsonObject() const;
  QString toString() const;

public:
  int id;
  QString name;
  QString mediaAddress; ///< Do not serialize, as long as we don't support P2P streaming.
  quint16 mediaPort; ///< Do not serialize, as long as we don't support P2P streaming.
};
Q_DECLARE_METATYPE(ClientEntity);
#endif