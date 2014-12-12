#ifndef QCORSERVER_HEADER
#define QCORSERVER_HEADER

#include <QTcpServer>
class QCorConnection;
class QCorFrame;

class QCorServer : public QTcpServer
{
  Q_OBJECT

public:
  QCorServer(QObject *parent);

protected:
  virtual void incomingConnection(qintptr socketDescriptor);

signals:
  void newConnection(QCorConnection *connection);
  void newFrame(QCorFrame *frame);
};

#endif