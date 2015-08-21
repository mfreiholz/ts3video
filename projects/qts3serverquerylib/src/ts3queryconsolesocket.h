#ifndef TS3QUERYCONSOLESOCKET_H
#define TS3QUERYCONSOLESOCKET_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QPair>
#include <QHash>
#include <QTcpSocket>
#include <QHostAddress>


class TS3QueryConsoleSocketSync : public QTcpSocket
{
  Q_OBJECT

public:
  TS3QueryConsoleSocketSync(QObject* parent = 0);
  virtual ~TS3QueryConsoleSocketSync();

  bool start(const QHostAddress& address, quint16 port);
  bool quit();

  bool login(const QString& loginName, const QString& loginPassword);
  bool useByPort(quint16 port);
  QList<QHash<QString, QString> > clientList();
  QList<QHash<QString, QString> > serverGroupsByClientId(quint64 clientDbId);

protected:
  QString socketReadLineSync();
};


class TS3QueryConsoleCommandData
{
public:
  QString cmd;
  QList<QPair<QString, QStringList> > params;
  QStringList options;
  QPair<int, QString> err;
  QList<QHash<QString, QString> > itemList;
};


#endif