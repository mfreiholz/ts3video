#ifndef HINTOVERLAYWIDGETPRIVATE_H
#define HINTOVERLAYWIDGETPRIVATE_H

#include "hintoverlaywidget.h"

class HintOverlayWidgetPrivate : public QObject
{
  Q_OBJECT

public:
  HintOverlayWidgetPrivate(HintOverlayWidget *o) :
    owner(o)
  {}

public:
  HintOverlayWidget *owner;
};

#endif