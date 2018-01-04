#ifndef QCORSERVERPRIVATE_HEADER
#define QCORSERVERPRIVATE_HEADER

#include <QObject>
class QCorServer;

class QCorServerPrivate : public QObject
{
  Q_OBJECT

public:
  QCorServerPrivate(QCorServer *owner);

public:
  QCorServer *owner;
};

#endif