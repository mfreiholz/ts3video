#ifndef ACTIONBASE_H
#define ACTIONBASE_H

#include <QRunnable>
#include <QJsonObject>
#include "qcorframe.h"
class QCorConnection;
class VirtualServer;
class ClientConnectionHandler;

///////////////////////////////////////////////////////////////////////

class ActionData
{
public:
  VirtualServer *server;
  ClientConnectionHandler *session;
  QCorConnection *connection;
  QCorFrameRefPtr frame;
  QString action;
  QJsonObject params;
};

///////////////////////////////////////////////////////////////////////

class ActionBase
{
public:
  //ActionData _req;

  enum Flag { NoFlag, RequiresAuthentication, RequiresAdminPrivileges };
  Q_DECLARE_FLAGS(Flags, Flag)

  virtual QString name() const = 0;
  virtual Flags flags() const { return RequiresAuthentication; }
  virtual void run(const ActionData &req) = 0;

  void sendDefaultOkResponse(const ActionData &req, const QJsonObject &params = QJsonObject());
  void sendDefaultErrorResponse(const ActionData &req, int statusCode, const QString &message);
  void broadcastNotificationToSiblingClients(const ActionData &req, const QString &action, const QJsonObject &params);
  void disconnectFromHostDelayed(const ActionData &req);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ActionBase::Flags)

typedef QSharedPointer<ActionBase> ActionPtr;

///////////////////////////////////////////////////////////////////////

class AuthenticationAction : public ActionBase
{
public:
  QString name() const { return QString("auth"); }
  Flags flags() const { return NoFlag; }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class GoodbyeAction : public ActionBase
{
public:
  QString name() const { return QString("goodbye"); }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class HeartbeatAction : public ActionBase
{
public:
  QString name() const { return QString("heartbeat"); }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class EnableVideoAction : public ActionBase
{
public:
  QString name() const { return QString("clientenablevideo"); }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class DisableVideoAction : public ActionBase
{
public:
  QString name() const { return QString("clientdisablevideo"); }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class EnableRemoteVideoAction : public ActionBase
{
public:
  QString name() const { return QString("enableremotevideo"); }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class DisableRemoteVideoAction : public ActionBase
{
public:
  QString name() const { return QString("disableremotevideo"); }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class JoinChannelAction : public ActionBase
{
public:
  QString name() const { return QString("joinchannel"); }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class JoinChannel2Action : public JoinChannelAction
{
public:
  QString name() const { return QString("joinchannelbyidentifier"); }
  void run(const ActionData &req) { JoinChannelAction::run(req); }
};

///////////////////////////////////////////////////////////////////////

class LeaveChannelAction : public ActionBase
{
public:
  QString name() const { return QString("leavechannel"); }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class AdminAuthAction : public ActionBase
{
public:
  QString name() const { return QString("adminauth"); }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

class KickClientAction : public ActionBase
{
public:
  QString name() const { return QString("kickclient"); }
  Flags flags() const { return RequiresAuthentication | RequiresAdminPrivileges; }
  void run(const ActionData &req);
};

///////////////////////////////////////////////////////////////////////

#endif
