#include "qwidgetutil.h"

#include <QDesktopWidget>
#include <QWidget>
#include <QRect>

namespace QWidgetUtil
{

void centerWidget(QWidget* widget)
{
	if (widget)
	{
		QDesktopWidget deskWidget;
		const auto screenIndex = deskWidget.primaryScreen();
		const auto deskRect = deskWidget.availableGeometry(screenIndex);
		const auto x = (deskRect.width() - widget->rect().width()) / 2;
		const auto y = (deskRect.height() - widget->rect().height()) / 2;
		widget->move(x, y);
	}
}

void resizeWidgetPerCent(QWidget* widget, qreal widthPC, qreal heightPC)
{
	if (widget && widthPC >= 0.0 && heightPC >= 0.0)
	{
		QDesktopWidget deskWidget;
		const auto screenIndex = deskWidget.primaryScreen();
		const auto deskRect = deskWidget.availableGeometry(screenIndex);
		const auto w = (deskRect.width() / 100) * widthPC;
		const auto h = (deskRect.height() / 100) * heightPC;
		widget->resize(w, h);
	}
}

}