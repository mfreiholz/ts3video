#ifndef HANGOUTVIEWWIDGETPRIVATE_H
#define HANGOUTVIEWWIDGETPRIVATE_H

#include "hangoutviewwidget.h"

class QFrame;
class QScrollArea;
class QBoxLayout;

class HangoutViewWidgetPrivate : public QObject
{
  Q_OBJECT

public:
  HangoutViewWidgetPrivate(HangoutViewWidget *o) : owner(o) {}
  void doCameraLayout();
  void doThumbnailLayout();
  
public:
  HangoutViewWidget *owner;

  QFrame *fullViewContainer;
  QScrollArea *thumbnailScrollArea;
  QFrame *thumbnailContainer;

  QWidget *cameraWidget;
  QList<QWidget*> thumbnailWidgets;
};

#endif