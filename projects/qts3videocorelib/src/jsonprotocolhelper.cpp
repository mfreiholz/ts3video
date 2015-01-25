#include "jsonprotocolhelper.h"

QByteArray JsonProtocolHelper::createJsonRequest(const QString &action, const QJsonObject &parameters)
{
  QJsonObject root;
  root["action"] = action;
  root["parameters"] = parameters;
  return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QByteArray JsonProtocolHelper::createJsonResponse(const  QJsonObject &data)
{
  QJsonObject root;
  root["status"] = 0;
  root["data"] = data;
  return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QByteArray JsonProtocolHelper::createJsonResponseError(int status, const QString &errorMessage)
{
  QJsonObject root;
  root["status"] = status;
  root["error"] = errorMessage;
  return QJsonDocument(root).toJson(QJsonDocument::Compact);
}