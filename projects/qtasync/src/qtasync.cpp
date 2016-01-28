#include "qtasync/qtasync.h"

#include <QFuture>
#include <QObject>
#include <QFutureWatcher>
#include <QtConcurrent>

QTASYNC_NS_BEGIN

void async(std::function<QVariant(void)> func, std::function<void(QVariant)> done)
{
  auto fw = new QFutureWatcher<QVariant>();
  QObject::connect(fw, &QFutureWatcher<QVariant>::finished, [=]()
  {
    fw->deleteLater();
    done(fw->result());
  });
  fw->setFuture(QtConcurrent::run(func));
}

QTASYNC_NS_END