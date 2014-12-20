#ifndef QTESTCLIENT_HEADER
#define QTESTCLIENT_HEADER

#include <QObject>
#include "qcorconnection.h"

class QTestClient : public QObject
{
  Q_OBJECT

public:
  QTestClient(QObject *parent);
  virtual ~QTestClient();

public slots:
  void connectToHost(const QHostAddress &address, quint16 port);

private slots:
  void onStateChanged(QAbstractSocket::SocketState state);
  void sendTestFrame();
  void onResponseFinished();

private:
  QCorConnection *_connection;
};

#endif