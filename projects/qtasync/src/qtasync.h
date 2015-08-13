#ifndef QTASYNC_H
#define QTASYNC_H

#define QTASYNC_NS_BEGIN namespace QtAsync {
#define QTASYNC_NS_END }

#include <functional>

#include <QVariant>

QTASYNC_NS_BEGIN

void async(std::function<QVariant(void)> func, std::function<void(QVariant)> done);

QTASYNC_NS_END
#endif