#include "App.hpp"
#include "Logging.hpp"
#include "libapp/cliententity.h"
#include <QtNetwork/QHostAddress>

App::App(QObject* parent)
	: QObject(parent)
	, m_networkClient(nullptr)
	, m_userInfoListModel(nullptr)
	, m_cameraVideoAdapter(nullptr)
{
	m_userInfoListModel.setClientListModel(m_networkClient.clientModel());
	QObject::connect(&m_networkClient, &NetworkClient::connected, this, &App::onNetworkClientConnected);
	QObject::connect(&m_networkClient, &NetworkClient::disconnected, this, &App::onNetworkClientDisconnected);
	QObject::connect(&m_networkClient, &NetworkClient::error, this, &App::onNetworkClientError);
	QObject::connect(&m_networkClient, &NetworkClient::mediaSocketAuthenticated, this, &App::onNetworkClientMediaSocketAuthenticated);
	QObject::connect(&m_networkClient, &NetworkClient::serverError, this, &App::onNetworkClientServerError);
	QObject::connect(&m_networkClient, &NetworkClient::clientEnabledVideo, this, &App::onClientEnabledVideo);
	QObject::connect(&m_networkClient, &NetworkClient::clientDisabledVideo, this, &App::onClientDisabledVideo);
	QObject::connect(&m_networkClient, &NetworkClient::clientJoinedChannel, this, &App::onClientJoinedChannel);
	QObject::connect(&m_networkClient, &NetworkClient::clientLeftChannel, this, &App::onClientLeftChannel);
	QObject::connect(&m_networkClient, &NetworkClient::clientKicked, this, &App::onClientKicked);
	QObject::connect(&m_networkClient, &NetworkClient::clientDisconnected, this, &App::onClientDisconnected);
	QObject::connect(&m_networkClient, &NetworkClient::newVideoFrame, this, &App::onNewVideoFrame);
	QObject::connect(&m_networkClient, &NetworkClient::networkUsageUpdated, this, &App::onNetworkUsageUpdated);

	QObject::connect(&m_cameraVideoAdapter, &CameraVideoAdapter::firstFrame, this, &App::onCameraVideoAdapterFirstFrame);
	QObject::connect(&m_cameraVideoAdapter, &CameraVideoAdapter::cameraEnabledChanged, this, &App::onCameraVideoAdapterVideoEnabledChanged);
	QObject::connect(&m_cameraVideoAdapter, &CameraVideoAdapter::newCameraImage, &m_networkClient, &NetworkClient::sendVideoFrame);
}

App::~App() = default;

void App::connectToServer(const QString& remoteAddress, int remotePort)
{
	qCDebug(logCore) << "connectToServer";

	auto sock = m_networkClient.socket();
	if (sock && sock->state() != QAbstractSocket::UnconnectedState)
		return;

	m_networkClient.connectToHost(QHostAddress(remoteAddress), remotePort);
}

void App::registerRemoteVideoAdapter(RemoteVideoAdapter* adapter)
{
	qCDebug(logCore, "registerRemoteVideoAdapter (clientId=%d)", adapter->getClientId());
	m_remoteVideoAdapters.insert(adapter->getClientId(), adapter);
}

void App::unregisterRemoteVideoAdapter(RemoteVideoAdapter* adapter)
{
	qCDebug(logCore, "unregisterRemoteVideoAdapter (clientId=%d)", adapter->getClientId());
	m_remoteVideoAdapters.remove(adapter->getClientId());
}

void App::onNetworkClientConnected()
{
	qCDebug(logCore) << "onNetworkClientConnected";
	auto reply = m_networkClient.auth("MyName", QString());
	QObject::connect(reply, &QCorReply::finished, [this]() {
		qCDebug(logCore) << "authentication succeed";
		auto reply = m_networkClient.joinChannelByIdentifier("default", "default");
		QObject::connect(reply, &QCorReply::finished, []() {
			qCDebug(logCore) << "joined channel";
		});
		QCorReply::autoDelete(reply);
	});
	QCorReply::autoDelete(reply);
}

void App::onNetworkClientDisconnected()
{
	qCDebug(logCore) << "onNetworkClientDisconnected";
}

void App::onNetworkClientError(QAbstractSocket::SocketError socketError)
{
	qCCritical(logCore) << "onNetworkClientError" << socketError;
}

void App::onNetworkClientMediaSocketAuthenticated()
{
	qCDebug(logCore) << "onNetworkClientMediaSocketAuthenticated";
}

void App::onNetworkClientServerError(int errorCode, const QString& errorMessage)
{
	qCCritical(logCore) << "onNetworkClientServerError" << errorCode << errorMessage;
}

void App::onClientEnabledVideo(const ClientEntity& client)
{
	qCDebug(logCore) << "onClientEnabledVideo";
}

void App::onClientDisabledVideo(const ClientEntity& client)
{
	qCDebug(logCore) << "onClientDisabledVideo";
}

void App::onClientJoinedChannel(const ClientEntity& client, const ChannelEntity& channel)
{
	qCDebug(logCore) << "onClientJoinedChannel";
}

void App::onClientLeftChannel(const ClientEntity& client, const ChannelEntity& channel)
{
	qCDebug(logCore) << "onClientLeftChannel";
}

void App::onClientKicked(const ClientEntity& client)
{
	qCDebug(logCore) << "onClientKicked";
}

void App::onClientDisconnected(const ClientEntity& client)
{
	qCDebug(logCore) << "onClientDisconnected";
}

void App::onNewVideoFrame(YuvFrameRefPtr frame, ocs::clientid_t senderId)
{
	//qCDebug(logCore, "Enter App::onNewVideoFrame(X, %d)", senderId);
	const auto it = m_remoteVideoAdapters.find(senderId);
	if (it == m_remoteVideoAdapters.cend())
		return;
	it->data()->setVideoFrame(frame);
	//qCDebug(logCore, "Leave App::onNewVideoFrame(X, %d)", senderId);
}

void App::onNetworkUsageUpdated(const NetworkUsageEntity& networkUsage)
{
	qCDebug(logCore) << "onNetworkUsageUpdated";
}

void App::onCameraVideoAdapterFirstFrame(int width, int height)
{
	qCDebug(logCore, "Enter onCameraVideoAdapterFirstFrame(%d, %d)", width, height);
	auto reply = m_networkClient.enableVideoStream(width, height, 350);
	QCorReply::autoDelete(reply);
	qCDebug(logCore, "Leave onCameraVideoAdapterFirstFrame(%d, %d)", width, height);
}

void App::onCameraVideoAdapterVideoEnabledChanged()
{
	qCDebug(logCore, "onCameraVideoAdapterVideoEnabledChanged");
	if (!m_cameraVideoAdapter.isCameraEnabled())
	{
		auto reply = m_networkClient.disableVideoStream();
		QCorReply::autoDelete(reply);
	}
}
