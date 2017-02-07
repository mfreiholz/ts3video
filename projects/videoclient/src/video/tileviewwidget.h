#ifndef TILEVIEWWIDGET_H
#define TILEVIEWWIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QCamera>

#include "libbase/defines.h"
#include "videolib/yuvframe.h"

class ConferenceVideoWindow;
class NetworkClient;
class ClientEntity;
class ChannelEntity;

class TileViewWidgetPrivate;
class TileViewWidget : public QWidget
{
	Q_OBJECT
	QScopedPointer<TileViewWidgetPrivate> d;

public:
	TileViewWidget(ConferenceVideoWindow* window, Qt::WindowFlags f = 0);
	virtual ~TileViewWidget();

	ConferenceVideoWindow* window() const;

	void addClient(const ClientEntity& client, const ChannelEntity& channel);
	void removeClient(const ClientEntity& client, const ChannelEntity& channel);
	void updateClientVideo(YuvFrameRefPtr frame, ocs::clientid_t senderId);

public slots:
	void increaseTileSize();
	void decreaseTileSize();
	void setTileSize(const QSize& size);

#if defined(OCS_INCLUDE_AUDIO)
	void setAudioInputEnabled(bool b);
#endif

protected:
	virtual void wheelEvent(QWheelEvent* e);
	virtual void showEvent(QShowEvent* e);
	virtual void hideEvent(QHideEvent* e);

private slots:
	void onClientEnabledVideo(const ClientEntity& c);
	void onClientDisabledVideo(const ClientEntity& c);
	void onCameraChanged();
	void onCameraStatusChanged(QCamera::Status s);
};

#endif
