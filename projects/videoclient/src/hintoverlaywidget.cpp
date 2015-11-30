#include "hintoverlaywidget_p.h"

HintOverlayWidget::HintOverlayWidget(QWidget* parent) :
	QFrame(parent),
	d(new HintOverlayWidgetPrivate(this))
{

}

HintOverlayWidget::~HintOverlayWidget()
{
}
