#include "hintoverlaywidget.h"

#include <QDebug>

#include <QtCore/QEvent>

#include <QtGui/QMouseEvent>

#include <QtWidgets/QBoxLayout>

QPointer<HintOverlayWidget> HintOverlayWidget::HINT;

HintOverlayWidget::HintOverlayWidget(QWidget* content, QWidget* target, QWidget* parent) :
	QFrame(parent),
	_content(content),
	_target(target)
{
	//setWindowFlags(windowFlags() | Qt::ToolTip);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

	auto l = new QBoxLayout(QBoxLayout::TopToBottom);
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(0);
	setLayout(l);

	l->addWidget(content);

	_content->setMouseTracking(true);
	_content->installEventFilter(this);

	_target->setMouseTracking(true);
	_target->installEventFilter(this);
}

HintOverlayWidget::~HintOverlayWidget()
{
}

void HintOverlayWidget::showHint(QWidget* content, QWidget* target)
{
	if (HINT)
		hideHint();

	HINT = new HintOverlayWidget(content, target, nullptr);

	auto pos = target->mapToGlobal(target->rect().topRight());
	HINT->resize(200, 500);
	HINT->move(pos);
	HINT->show();
}

void HintOverlayWidget::hideHint()
{
	if (!HINT)
		return;
	HINT->deleteLater();
	HINT.clear();
}

bool HintOverlayWidget::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == _target)
	{
		switch (e->type())
		{
		case QEvent::MouseMove:
		{
			QMouseEvent* ev = static_cast<QMouseEvent*>(e);
			qDebug() << ev->globalPos();
		}
		}
	}
	return true;
}