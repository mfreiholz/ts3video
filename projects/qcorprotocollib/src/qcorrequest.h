#ifndef QCORREQUEST_HEADER
#define QCORREQUEST_HEADER

#include <QObject>
class QCorConnection;

class QCorRequest : public QObject
{
  Q_OBJECT

public:
  QCorRequest(QCorConnection *connection, QObject *parent);
  QCorConnection* connection() const;

private:
  QCorConnection *_connection;
};

#endif