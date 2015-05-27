#ifndef ABSTRACTMODULEBASE_H
#define ABSTRACTMODULEBASE_H

#include <exception>
#include <QSet>
#include <QString>
#include <QJsonObject>
#include "qcorframe.h"
class VirtualServer;
class ClientConnectionHandler;

class AbstractModuleBase
{
public:
  class ActionRequest
  {
  public:
    VirtualServer *server;
    ClientConnectionHandler *connection;
    QCorFrameRefPtr frame;
    QString action;
    QJsonObject params;

    ActionRequest(VirtualServer *s, ClientConnectionHandler *c, QCorFrameRefPtr f, const QString &a, const QJsonObject &p) :
      server(s),
      connection(c),
      frame(f),
      action(a),
      params(p)
    {}
  };

  /*!
    Provides a list of unique actions the implementation of this module supports.
    \return
  */
  virtual QSet<QString> actions() const = 0;
  
  /*!
    \return 0 = OK; Everything else indicates an error.
    \throws ActionRequestException
  */
  virtual int processActionRequest(const ActionRequest &req) = 0;

};


class ActionRequestException : public std::exception
{
public:
  ActionRequestException(const AbstractModuleBase::ActionRequest &request) {}
  QString getMessage() const {}
};

#endif
