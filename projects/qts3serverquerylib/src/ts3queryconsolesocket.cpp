#include "ts3queryconsolesocket.h"
#include "ts3serverquery.h"

#include "humblelogging/api.h"

HUMBLE_LOGGER(HL, "ts3");

#define LOG_RESPONSE_ERROR(LOGGER, CMD, LINE) HL_ERROR(LOGGER, QString("Error response (command=%1; response=%2)").arg(CMD).arg(LINE).toStdString())


TS3QueryConsoleSocketSync::TS3QueryConsoleSocketSync(QObject* parent) :
  QTcpSocket(parent)
{

}


TS3QueryConsoleSocketSync::~TS3QueryConsoleSocketSync()
{
  if (state() == QTcpSocket::ConnectedState)
    quit();
}


bool TS3QueryConsoleSocketSync::start(const QHostAddress& address, quint16 port)
{
  connectToHost(address, port);
  if (!waitForConnected())
  {
    HL_ERROR(HL, QString("Can not connect to query-console (address=%1; port=%2)").arg(address.toString()).arg(port).toStdString());
    return false;
  }

  // Verify "hello" from server.
  auto line = socketReadLineSync();
  if (line.compare("TS3") != 0)
  {
    HL_ERROR(HL, QString("Invalid welcome-message (msg=%1)").arg(line).toStdString());
    return false;
  }

  // Read additional welcome message.
  // TODO Is it optional?
  line = socketReadLineSync();

  return true;
}


bool TS3QueryConsoleSocketSync::quit()
{
  TS3ServerQuery sq;
  TS3QueryConsoleCommandData c;
  c.cmd = sq.createCommand("quit", c.params, c.options);
  write(c.cmd.toUtf8());

  disconnectFromHost();
  if (state() != QTcpSocket::UnconnectedState)
  {
    waitForDisconnected();
  }
  return true;
}


bool TS3QueryConsoleSocketSync::login(const QString& loginName, const QString& loginPassword)
{
  TS3ServerQuery sq;
  TS3QueryConsoleCommandData c;
  c.params << qMakePair(QString("client_login_name"), QStringList() << loginName);
  c.params << qMakePair(QString("client_login_password"), QStringList() << loginPassword);
  c.cmd = sq.createCommand("login", c.params, c.options);
  write(c.cmd.toUtf8());

  auto line = socketReadLineSync();
  c.err = sq.parseError(line);
  if (c.err.first != 0)
  {
    LOG_RESPONSE_ERROR(HL, c.cmd, line);
    return false;
  }
  return true;
}


bool TS3QueryConsoleSocketSync::useByPort(quint16 port)
{
  TS3ServerQuery sq;
  TS3QueryConsoleCommandData c;
  c.params << qMakePair(QString("port"), QStringList() << QString::number(port));
  c.cmd = sq.createCommand("use", c.params, c.options);
  write(c.cmd.toUtf8());

  auto line = socketReadLineSync();
  c.err = sq.parseError(line);
  if (c.err.first != 0)
  {
    LOG_RESPONSE_ERROR(HL, c.cmd, line);
    return false;
  }
  return true;
}


QList<QHash<QString, QString> > TS3QueryConsoleSocketSync::clientList()
{
  TS3ServerQuery sq;
  TS3QueryConsoleCommandData c;
  c.options << "-ip";
  c.cmd = sq.createCommand("clientlist", c.params, c.options);
  write(c.cmd.toUtf8());

  auto line = socketReadLineSync();
  if (sq.isErrorLine(line))
  {
    c.err = sq.parseError(line);
    LOG_RESPONSE_ERROR(HL, c.cmd, line);
    return c.itemList;
  }
  c.itemList = sq.parseItemList(line);

  // Parse error line.
  line = socketReadLineSync();
  c.err = sq.parseError(line);
  if (c.err.first != 0)
  {
    LOG_RESPONSE_ERROR(HL, c.cmd, line);
    return c.itemList;
  }

  return c.itemList;
}


QList<QHash<QString, QString> > TS3QueryConsoleSocketSync::serverGroupsByClientId(quint64 clientDbId)
{
  TS3ServerQuery sq;
  TS3QueryConsoleCommandData c;
  c.params << qMakePair(QString("cldbid"), QStringList() << QString::number(clientDbId));
  c.cmd = sq.createCommand("servergroupsbyclientid", c.params, c.options);
  write(c.cmd.toUtf8());

  auto line = socketReadLineSync();
  if (sq.isErrorLine(line))
  {
    c.err = sq.parseError(line);
    LOG_RESPONSE_ERROR(HL, c.cmd, line);
    return c.itemList;
  }
  c.itemList = sq.parseItemList(line);

  // Parse error line.
  line = socketReadLineSync();
  c.err = sq.parseError(line);
  if (c.err.first != 0)
  {
    LOG_RESPONSE_ERROR(HL, c.cmd, line);
    return c.itemList;
  }

  return c.itemList;
}


QString TS3QueryConsoleSocketSync::socketReadLineSync()
{
  while (!canReadLine())
  {
    waitForReadyRead();
  }
  return QString::fromUtf8(readLine()).trimmed();
}