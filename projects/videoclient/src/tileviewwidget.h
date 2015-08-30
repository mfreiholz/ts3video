#ifndef TILEVIEWWIDGET_H
#define TILEVIEWWIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include "viewbase.h"

class TileViewWidgetPrivate;
class TileViewWidget : public QWidget, public ViewBase
{
	Q_OBJECT
	QScopedPointer<TileViewWidgetPrivate> d;

public:
	TileViewWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
	virtual ~TileViewWidget();

	virtual void setClientListModel(ClientListModel* model);
	virtual void setCamera(const QSharedPointer<QCamera>& c);
	virtual void addClient(const ClientEntity& client, const ChannelEntity& channel);
	virtual void removeClient(const ClientEntity& client, const ChannelEntity& channel);
	virtual void updateClientVideo(YuvFrameRefPtr frame, int senderId);

public slots:
	void setTileSize(const QSize& size);
	void setVideoEnabled(bool b);

protected:
	virtual void wheelEvent(QWheelEvent* e);
	virtual void showEvent(QShowEvent* e);
	virtual void hideEvent(QHideEvent* e);

private slots:
	void onClientEnabledVideo(const ClientEntity& c);
	void onClientDisabledVideo(const ClientEntity& c);
};

#endif