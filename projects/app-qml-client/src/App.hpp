#pragma once
#include "CameraVideoAdapter.hpp"
#include "RemoteVideoAdapter.hpp"
#include "UserInfoListModel.hpp"
#include "libclient/networkclient/networkclient.h"
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QPointer>

class App : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString cameraDeviceId MEMBER m_cameraDeviceId NOTIFY cameraDeviceIdChanged)
	Q_PROPERTY(UserInfoListModel* userInfoList READ getUserInfoListModel CONSTANT)
	Q_PROPERTY(CameraVideoAdapter* cameraVideoAdapter READ getCameraVideoAdapter CONSTANT)

public:
	explicit App(QObject* parent);
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App() override;

	Q_INVOKABLE void connectToServer();

	NetworkClient* getNetworkClient() { return &m_networkClient; }
	UserInfoListModel* getUserInfoListModel() { return &m_userInfoListModel; }
	CameraVideoAdapter* getCameraVideoAdapter() { return &m_cameraVideoAdapter; }

	Q_INVOKABLE void registerRemoteVideoAdapter(RemoteVideoAdapter* adapter);
	Q_INVOKABLE void unregisterRemoteVideoAdapter(RemoteVideoAdapter* adapter);

signals:
	void cameraDeviceIdChanged();
	void connected();
	void disconnected();

private:
	QString m_cameraDeviceId;
	NetworkClient m_networkClient;
	UserInfoListModel m_userInfoListModel;
	CameraVideoAdapter m_cameraVideoAdapter;
	QMap<int, QPointer<RemoteVideoAdapter>> m_remoteVideoAdapters;

	Q_SLOT void onNetworkClientConnected();
	Q_SLOT void onNetworkClientDisconnected();
	Q_SLOT void onNetworkClientError(QAbstractSocket::SocketError socketError);
	Q_SLOT void onNetworkClientMediaSocketAuthenticated();
	Q_SLOT void onNetworkClientServerError(int errorCode, const QString& errorMessage);
	Q_SLOT void onClientEnabledVideo(const ClientEntity& client);
	Q_SLOT void onClientDisabledVideo(const ClientEntity& client);
	Q_SLOT void onClientJoinedChannel(const ClientEntity& client, const ChannelEntity& channel);
	Q_SLOT void onClientLeftChannel(const ClientEntity& client, const ChannelEntity& channel);
	Q_SLOT void onClientKicked(const ClientEntity& client);
	Q_SLOT void onClientDisconnected(const ClientEntity& client);
	Q_SLOT void onNewVideoFrame(YuvFrameRefPtr frame, ocs::clientid_t senderId);
	Q_SLOT void onNetworkUsageUpdated(const NetworkUsageEntity& networkUsage);

	Q_SLOT void onCameraVideoAdapterFirstFrame(int width, int height);
	Q_SLOT void onCameraVideoAdapterVideoEnabledChanged();
};
