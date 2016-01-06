#ifndef TILEVIEWWIDGET_H
#define TILEVIEWWIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QCamera>

#include "viewbase.h"

class ConferenceVideoWindow;

class TileViewWidgetPrivate;
class TileViewWidget : public QWidget, public ViewBase
{
	Q_OBJECT
	QScopedPointer<TileViewWidgetPrivate> d;

public:
	TileViewWidget(ConferenceVideoWindow* window, QWidget* parent = 0, Qt::WindowFlags f = 0);
	virtual ~TileViewWidget();
	ConferenceVideoWindow* window() const
	{
		return _window;
	}

	virtual void addClient(const ClientEntity& client, const ChannelEntity& channel);
	virtual void removeClient(const ClientEntity& client, const ChannelEntity& channel);
	virtual void updateClientVideo(YuvFrameRefPtr frame, int senderId);

public slots:
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

private:
	ConferenceVideoWindow* _window;
	QSharedPointer<QCamera> _camera;
};

#endif