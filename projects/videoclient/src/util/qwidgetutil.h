#ifndef QWIDGETUTIL_H
#define QWIDGETUTIL_H

#include <QtGlobal>
class QWidget;

namespace QWidgetUtil
{

void centerWidget(QWidget* widget);
void resizeWidgetPerCent(QWidget* widget, qreal widthPC, qreal heightPC);

}

#endif