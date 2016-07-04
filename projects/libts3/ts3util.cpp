#include "ts3util.h"
#include "ts3serverquery.h"

#include <QString>
#include <QHostAddress>
#include <QTcpSocket>

#include "humblelogging/api.h"

HUMBLE_LOGGER(HL, "ts3");

namespace TS3Util
{

bool isClientConnected(const QHostAddress& address, quint16 port, const QString& loginName, const QString& loginPassword, quint16 vsPort, const QString& clientIp)
{
  TS3ServerQuery sq;

  QTcpSocket socket;
  socket.connectToHost(address, port);
  if (!socket.waitForConnected())
  {
    HL_ERROR(HL, QString("Can not connect to query-console (address=%1; port=%2)").arg(address.toString()).arg(port).toStdString());
    return false;
  }

  // Verify welcome message.
  QString line;
  if ((line = socketReadLineSync(socket)) != "TS3")
  {
    HL_ERROR(HL, QString("Invalid welcome-message (msg=%1)").arg(line).toStdString());
    socket.close();
    return false;
  }
  line = socketReadLineSync(socket); // Welcome text...

  QString cmd;
  QList<QPair<QString, QStringList> > params;
  QStringList options;
  QPair<int, QString> err;

  // Login.
  params.clear();
  params << qMakePair(QString("client_login_name"), QStringList() << loginName);
  params << qMakePair(QString("client_login_password"), QStringList() << loginPassword);
  options.clear();
  cmd = sq.createCommand("login", params, options);
  socket.write(cmd.toUtf8());

  line = socketReadLineSync(socket);
  err = sq.parseError(line);
  if (err.first != 0)
  {
    HL_ERROR(HL, QString("Login failed (name=%1)").arg(loginName).toStdString());
    socket.close();
    return false;
  }

  // Select virtual server.
  params.clear();
  params << qMakePair(QString("port"), QStringList() << QString::number(vsPort));
  options.clear();
  cmd = sq.createCommand("use", params, options);
  socket.write(cmd.toUtf8());

  line = socketReadLineSync(socket);
  err = sq.parseError(line);
  if (err.first != 0)
  {
    HL_ERROR(HL, QString("Can not select virtual server (%1)").arg(line).toStdString());
    socket.close();
    return false;
  }

  // Get client list.
  params.clear();
  options.clear();
  options << "-ip";
  cmd = sq.createCommand("clientlist", params, options);
  socket.write(cmd.toUtf8());

  line = socketReadLineSync(socket);
  auto clientList = sq.parseItemList(line);
  line = socketReadLineSync(socket);
  err = sq.parseError(line);
  if (err.first != 0)
  {
    HL_ERROR(HL, QString("Can not retrieve client list (%1)").arg(line).toStdString());
    socket.close();
    return false;
  }

  // Quit (Close connection).
  params.clear();
  options.clear();
  cmd = sq.createCommand("quit", params, options);
  socket.write(cmd.toUtf8());

  socket.disconnectFromHost();
  if (socket.state() != QTcpSocket::UnconnectedState)
  {
    socket.waitForDisconnected();
  }

  // Find client by IP.
  int found = -1;
  for (auto i = 0; i < clientList.size(); ++i)
  {
    const auto& item = clientList[i];
    const auto type = item.value("client_type").toInt();
    const auto ip = item.value("connection_client_ip");
    if (type == 0 && ip.compare(clientIp) == 0)
    {
      found = i;
      break;
    }
  }
  return (found >= 0);
  //printf("\nClient list:\n");
  //foreach (auto item, clientList)
  //{
  //  printf("\tClient (db-id=%s; type=%s; ip=%s)\n",
  //         item.value("client_database_id").toStdString().c_str(),
  //         item.value("client_type").toStdString().c_str(),
  //         item.value("connection_client_ip").toStdString().c_str()
  //        );
  //}
}


QByteArray socketReadLineSync(QTcpSocket& s)
{
  while (!s.canReadLine())
  {
    s.waitForReadyRead();
  }
  return s.readLine().trimmed();
}

}