#pragma once

#include <QScopedPointer>
#include <QWidget>
#include <QFrame>
#include <QCamera>

#include "libbase/defines.h"
#include "libapp/yuvframe.h"

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

public slots:
	void onTileMoveBackward();
	void onTileMoveForward();

private slots:
	void onClientEnabledVideo(const ClientEntity& c);
	void onClientDisabledVideo(const ClientEntity& c);
	void onCameraChanged();
	void onCameraStatusChanged(QCamera::Status s);
};


/*
	Basic wrapper widget for all visible tiles (camera, remote-video, ...)
*/
class TileViewTileFrame :
	public QFrame
{
	Q_OBJECT
	class Private;
	QScopedPointer<Private> d;

public:
	TileViewTileFrame(TileViewWidget* tileView, QWidget* parent = nullptr);
	virtual ~TileViewTileFrame();

	void setWidget(QWidget* widget);
	QWidget* widget() const;

	void setClientInfo(const ClientEntity& client);

signals:
	void moveBackwardClicked();
	void moveForwardClicked();
};


/*
	Widget to manage the local user's camera video.
*/
class TileViewCameraWidget :
	public QFrame
{
	Q_OBJECT
	friend class TileViewWidget;

public:
	TileViewCameraWidget(TileViewWidget* tileView, QWidget* parent = nullptr);

private slots:
	void onCameraChanged();

private:
	TileViewWidget* _tileView;
	class QBoxLayout* _mainLayout;
	class ClientCameraVideoWidget* _cameraWidget;
};


/*
	Widget to display the video of a remote user.
*/
class TileViewTileWidget :
	public QFrame
{
	Q_OBJECT
	friend class TileViewWidget;

public:
	TileViewTileWidget(TileViewWidget* tileView, const ClientEntity& client,
					   QWidget* parent = nullptr);

private:
	TileViewWidget* _tileView;
	class RemoteClientVideoWidget* _videoWidget;
};