#ifndef HINTOVERLAYWIDGET_H
#define HINTOVERLAYWIDGET_H

#include <QtCore/QPointer>

#include <QtWidgets/QFrame>

class HintOverlayWidgetPrivate;
class HintOverlayWidget : public QFrame
{
	Q_OBJECT

public:
	HintOverlayWidget(QWidget* content, QWidget* target, QWidget* parent);
	virtual ~HintOverlayWidget();

	static QPointer<HintOverlayWidget> HINT;
	static void showHint(QWidget* content, QWidget* target);
	static void hideHint();

protected:
	virtual bool eventFilter(QObject*, QEvent*);

private:
	QWidget* _content;
	QWidget* _target;
};

#endif