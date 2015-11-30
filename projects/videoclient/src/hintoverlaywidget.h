#ifndef HINTOVERLAYWIDGET_H
#define HINTOVERLAYWIDGET_H

#include <QScopedPointer>
#include <QFrame>

class HintOverlayWidgetPrivate;
class HintOverlayWidget : public QFrame
{
	Q_OBJECT
	friend class HintOverlayWidgetPrivate;
	QScopedPointer<HintOverlayWidgetPrivate> d;

public:
	HintOverlayWidget(QWidget* parent = 0);
	virtual ~HintOverlayWidget();
};

#endif