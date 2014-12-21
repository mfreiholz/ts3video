#ifndef QCORSERVER_HEADER
#define QCORSERVER_HEADER

#include <QTcpServer>
class QCorConnection;

class QCorServer : public QTcpServer
{
  Q_OBJECT

public:
  QCorServer(QObject *parent);
  virtual ~QCorServer();

signals:
  /* Emits for every new connection.
   * The ownership goes over to the receiver.
   */
  void newConnection(QCorConnection *connection);

protected:
  virtual void incomingConnection(qintptr socketDescriptor);

private:
  friend class QCorServerPrivate;
  QCorServerPrivate *d;
};

#endif