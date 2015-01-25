#ifndef JSONPROTOCOLHELPER_H
#define JSONPROTOCOLHELPER_H

#include <QByteArray>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

class JsonProtocolHelper
{
public:
  static QByteArray createJsonRequest(const QString &action, const QJsonObject &parameters);
  static QByteArray createJsonResponse(const  QJsonObject &data);
  static QByteArray createJsonResponseError(int status, const QString &errorMessage = QString());

  static bool fromJsonRequest(const QByteArray &data, QString &action, QJsonObject &parameters);
};

#endif