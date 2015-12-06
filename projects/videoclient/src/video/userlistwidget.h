#ifndef CONFERENCEVIDEO_USERLISTWIDGET_H
#define CONFERENCEVIDEO_USERLISTWIDGET_H

#include <QtWidgets/QFrame>
class QString;
class QPoint;
class QListView;

class ConferenceVideoWindow;
class ClientListModel;

class UserListWidget : public QFrame
{
	Q_OBJECT

public:
	UserListWidget(ConferenceVideoWindow* window, QWidget* parent);

private slots:
	void onFilterTextChanged(const QString& text);
	void onCustomContextMenuRequested(const QPoint& point);

private:
	ConferenceVideoWindow* _window;
	QListView* _listView;
};

#endif