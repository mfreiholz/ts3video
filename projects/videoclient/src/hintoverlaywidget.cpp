#include "hintoverlaywidget.h"

#include <QtCore/QEvent>

#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QGraphicsDropShadowEffect>

QPointer<HintOverlayWidget> HintOverlayWidget::HINT;

HintOverlayWidget::HintOverlayWidget(QWidget* content, QWidget* target, QWidget* parent) :
	QFrame(parent, Qt::Tool | Qt::FramelessWindowHint),
	_content(content),
	_target(target)
{
	auto l = new QBoxLayout(QBoxLayout::TopToBottom);
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(0);
	setLayout(l);

	l->addWidget(content);

	qApp->installEventFilter(this);
}

HintOverlayWidget::~HintOverlayWidget()
{
	qApp->removeEventFilter(this);
}

QWidget* HintOverlayWidget::showHint(QWidget* content, QWidget* target)
{
	if (HINT)
	{
		hideHint();
	}

	HINT = new HintOverlayWidget(content, target, target);

	auto pos = target->mapToGlobal(target->rect().topRight());
	HINT->move(pos);
	HINT->show();
	return HINT;
}

void HintOverlayWidget::hideHint()
{
	if (!HINT)
	{
		return;
	}
	HINT->deleteLater();
	HINT.clear();
}

bool HintOverlayWidget::eventFilter(QObject* obj, QEvent* e)
{
	switch (e->type())
	{
		case QEvent::MouseMove:
		{
			auto ev = static_cast<QMouseEvent*>(e);
			auto gpos = ev->globalPos();
			if (!_target->rect().contains(_target->mapFromGlobal(gpos)) && !_content->rect().contains(_content->mapFromGlobal(gpos)))
			{
				hideHint();
			}
			break;
		}
	}
	return QFrame::eventFilter(obj, e);
}