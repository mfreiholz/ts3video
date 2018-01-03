#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "gridviewwidgetarranger.h"

GridViewWidgetArranger::GridViewWidgetArranger(QObject *parent) :
  QObject(parent),
  _animationsEnabled(true),
  _columnCount(1),
  _columnSpacing(0)
{
  auto desk = QApplication::desktop();
  auto screenIndex = desk->primaryScreen();
  screenIndex = desk->screenNumber(QCursor::pos());
  _rect = desk->availableGeometry(screenIndex);

  for (auto i = 0; i < desk->screenCount(); ++i) {
    auto geometry = desk->availableGeometry(i);
    qDebug() << QString("screen=%1; x1=%2; y1=%3; w=%4; h=%5")
      .arg(i)
      .arg(geometry.x())
      .arg(geometry.y())
      .arg(geometry.width())
      .arg(geometry.height());
  }
}

void GridViewWidgetArranger::arrange()
{
  // Grid properties.
  auto columnCount = _columnCount;
  auto columnSpacing = _columnSpacing;

  // Grid geometry.
  auto r = _rect;
  if (columnSpacing > 0) {
    r = r.adjusted(columnSpacing, columnSpacing, -columnSpacing, -columnSpacing);
  }

  // Grid cell size.
  auto widgetWidth = r.width() / columnCount;
  if (columnSpacing > 0) {
    auto rw = r.width();
    rw -= columnCount * columnSpacing;
    widgetWidth = rw / columnCount;
  }

  auto widgetHeight = widgetWidth;
  if (widgetHeight > r.height()) {
    widgetHeight = r.height();
  }

  // Calculate new widget geometries.
  QList<QRect> geometries;
  auto x = r.x();
  auto y = r.y();
  for (auto i = 0; i < _widgets.size(); ++i) {
    auto widgetGeometry = QRect(x, y, widgetWidth, widgetHeight);
    geometries.append(widgetGeometry);

    x += widgetWidth + columnSpacing;
    if (x + widgetWidth > r.x() + r.width()) {
      x = r.x();
      y += widgetHeight + columnSpacing;
    }
  }

  // DEBUG
  foreach (auto geometry, geometries) {
    qDebug() << QString("x=%1; y=%2; w=%3; h=%4")
      .arg(geometry.x())
      .arg(geometry.y())
      .arg(geometry.width())
      .arg(geometry.height());
  }

  // Apply the new geometries.
  if (_animationsEnabled) {
    auto animGroup = new QParallelAnimationGroup(this);
    for (auto i = 0; i < _widgets.size(); ++i) {
      auto widget = _widgets[i];
      auto anim = new QPropertyAnimation(widget, "geometry");
      anim->setDuration(150);
      anim->setStartValue(widget->geometry());
      anim->setEndValue(geometries[i]);
      animGroup->addAnimation(anim);
    }
    animGroup->start(QAbstractAnimation::DeleteWhenStopped);
  }
  // Apply geometries without animations.
  else {
    for (auto i = 0; i < _widgets.size(); ++i) {
      auto widget = _widgets[i];
      widget->setGeometry(geometries[i]);
    }
  }

//  auto w = new QWidget(0);
//  w->setWindowFlags(Qt::FramelessWindowHint);
//  w->setGeometry(r);
//  w->show();
}
