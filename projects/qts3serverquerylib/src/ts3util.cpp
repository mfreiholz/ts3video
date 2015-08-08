#include "ts3util.h"
#include "ts3serverquery.h"

#include <cstdio>

#include <QString>
#include <QHostAddress>
#include <QTcpSocket>

namespace TS3Util
{

bool isClientConnected(const QHostAddress& address, quint16 port, const QString& loginName, const QString& loginPassword, quint16 vsPort, const QString& clientIp)
{
  TS3ServerQuery sq;

  QTcpSocket socket;
  socket.connectToHost(address, port);
  if (!socket.waitForConnected())
  {
    printf("can not connect to query-console.");
    return false;
  }

  // Verify welcome message.
  QString line;
  if ((line = socketReadLineSync(socket)) != "TS3")
  {
    printf("invalid welcome from query-console: %s", line.toStdString().c_str());
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
    printf("login failed (%s)", line.toStdString().c_str());
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
    printf("clientlist failed: %s", line.toStdString().c_str());
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