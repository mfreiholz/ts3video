#ifndef QCORSERVER_HEADER
#define QCORSERVER_HEADER

#include <QTcpServer>

class QCorServer : public QTcpServer
{
  Q_OBJECT

public:
  QCorServer(QObject *parent);
};

#endif