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
  int id; ///< The client's ID assigned by server.
  QString name; ///< Visible name of the client.
  QString mediaAddress; ///< Do not serialize, as long as we don't support P2P streaming.
  quint16 mediaPort; ///< Do not serialize, as long as we don't support P2P streaming.
  
  // Video settings.
  bool videoEnabled; ///< Indicates whether the client has video enabled.
};
Q_DECLARE_METATYPE(ClientEntity);
#endif