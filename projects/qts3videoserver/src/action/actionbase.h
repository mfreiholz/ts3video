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
  ActionData _req;

  enum Flag { NoFlag, RequiresAuthentication, RequiresAdminPrivileges };
  Q_DECLARE_FLAGS(Flags, Flag)

  virtual QString name() const = 0;
  virtual Flags flags() const { return RequiresAuthentication; }
  virtual void run() = 0;

  void sendDefaultOkResponse(const QJsonObject &params = QJsonObject());
  void sendDefaultErrorResponse(int statusCode = 500, const QString &message = QString());
  void broadcastNotificationToSiblingClients(const QString &action, const QJsonObject &params);
  void disconnectFromHostDelayed();
};

typedef QSharedPointer<ActionBase> ActionPtr;

///////////////////////////////////////////////////////////////////////

class AuthenticationAction : public ActionBase
{
public:
  QString name() const { return QString("auth"); }
  Flags flags() const { return NoFlag; }
  void run();
};

///////////////////////////////////////////////////////////////////////

class GoodbyeAction : public ActionBase
{
public:
  QString name() const { return QString("goodbye"); }
  void run();
};

///////////////////////////////////////////////////////////////////////

class HeartbeatAction : public ActionBase
{
public:
  QString name() const { return QString("heartbeat"); }
  void run();
};

///////////////////////////////////////////////////////////////////////

class EnableVideoAction : public ActionBase
{
public:
  QString name() const { return QString("clientenablevideo"); }
  void run();
};

///////////////////////////////////////////////////////////////////////

class DisableVideoAction : public ActionBase
{
public:
  QString name() const { return QString("clientdisablevideo"); }
  void run();
};

///////////////////////////////////////////////////////////////////////

class EnableRemoteVideoAction : public ActionBase
{
public:
  QString name() const { return QString("enableremotevideo"); }
  void run();
};

///////////////////////////////////////////////////////////////////////

class DisableRemoteVideoAction : public ActionBase
{
public:
  QString name() const { return QString("disableremotevideo"); }
  void run();
};

///////////////////////////////////////////////////////////////////////

class JoinChannelAction : public ActionBase
{
public:
  QString name() const { return QString("joinchannel"); }
  void run();
};

///////////////////////////////////////////////////////////////////////

class JoinChannel2Action : public JoinChannelAction
{
public:
  QString name() const { return QString("joinchannelbyidentifier"); }
  void run() { JoinChannelAction::run(); }
};

///////////////////////////////////////////////////////////////////////

class LeaveChannelAction : public ActionBase
{
public:
  QString name() const { return QString("leavechannel"); }
  void run();
};

///////////////////////////////////////////////////////////////////////

class AdminAuthAction : public ActionBase
{
public:
  QString name() const { return QString("adminauth"); }
  void run();
};

///////////////////////////////////////////////////////////////////////

class KickClientAction : public ActionBase
{
public:
  QString name() const { return QString("kickclient"); }
  Flags flags() const { return RequiresAuthentication | RequiresAdminPrivileges; }
  void run();
};

///////////////////////////////////////////////////////////////////////

#endif
