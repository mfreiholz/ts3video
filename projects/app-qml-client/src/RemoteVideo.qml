import QtQuick 2.15
import QtMultimedia 5.15
import RemoteVideoAdapter 1.0

Item {
	id: root
	property alias remoteClientId: adapter.clientId

	RemoteVideoAdapter {
		id: adapter
	}

	VideoOutput {
		id: videoOutput
		anchors.fill: parent
		fillMode: VideoOutput.PreserveAspectFit
		source: adapter
	}

	Component.onCompleted: {
		console.debug("Enter RemoteVideo.onCompleted() - " + clientId)
		app.registerRemoteVideoAdapter(adapter)
		console.debug("Leave RemoteVideo.onCompleted() - " + clientId)
	}

	Component.onDestruction: {
		console.debug("Enter RemoteVideo.onDestruction() - " + clientId)
		app.unregisterRemoteVideoAdapter(adapter)
		console.debug("Leave RemoteVideo.onDestruction() - " + clientId)
	}

	onRemoteClientIdChanged: {
		console.debug("Enter RemoteVideo.onRemoteClientIdChanged() - " + clientId)
		console.debug("Leave RemoteVideo.onRemoteClientIdChanged() - " + clientId)
	}
}
