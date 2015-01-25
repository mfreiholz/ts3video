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

bool JsonProtocolHelper::fromJsonRequest(const QByteArray &data, QString &action, QJsonObject &parameters)
{
  QJsonParseError err;
  auto doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    return false;
  }
  else if (!doc.isObject()) {
    return false;
  }
  auto root = doc.object();
  if (!root.contains("action") || root["action"].toString().isEmpty() || !root.contains("parameters")) {
    return false;
  }
  action = root["action"].toString();
  parameters = root["parameters"].toObject();
  return true;
}