#include "ts3util.h"
#include "ts3serverquery.h"

#include <cstdio>

#include <QString>
#include <QHostAddress>
#include <QTcpSocket>

namespace TS3Util
{

bool isClientConnected(const QString& ip)
{
  // login serveradmin TiHxQDHt
  // use 9987
  // clientlist -ip
  // quit
  TS3ServerQuery sq;

  QTcpSocket socket;
  socket.connectToHost(QHostAddress::LocalHost, 10011);
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
  params << qMakePair(QString("client_login_name"), QStringList() << "serveradmin");
  params << qMakePair(QString("client_login_password"), QStringList() << "TiHxQDHt");
  options.clear();
  cmd = sq.createCommand("login", params, options);
  socket.write(cmd.toUtf8());

  line = socketReadLineSync(socket);
  err = sq.parseError(line);
  if (err.first != 0)
  {
    socket.close();
    return false;
  }

  // Select virtual server.
  params.clear();
  params << qMakePair(QString("port"), QStringList() << "9987");
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
    socket.waitForDisconnected();

  printf("\nClient list:\n");
  foreach (auto item, clientList)
  {
    printf("\tClient (db-id=%s; type=%s; ip=%s)\n",
           item.value("client_database_id").toStdString().c_str(),
           item.value("client_type").toStdString().c_str(),
           item.value("connection_client_ip").toStdString().c_str()
          );
  }

  return true;
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