#ifndef QCORREQUEST_HEADER
#define QCORREQUEST_HEADER

#include <QObject>
class QCorConnection;
class QCorRequest;

class QCorResponse : public QObject
{
  Q_OBJECT

public:
  QCorResponse(QCorConnection *connection, QCorRequest *request, QObject *parent);

private:
  QCorConnection *_connection;
  QCorRequest *_request;
};

#endif