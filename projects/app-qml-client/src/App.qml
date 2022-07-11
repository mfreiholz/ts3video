import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
	id: window
	visible: true
	title: qsTr("Hello World")
	height: 720
	width: 1280

	Component.onCompleted: {
	}

	Drawer {
		id: drawer
		width: window.width * 0.33
		height: window.height

		AppSettings {
			anchors.fill: parent
		}
	}

	Loader {
		id: loader
		anchors.fill: parent
		sourceComponent: connectionInfoComponent
	}

	Component {
		id: homeComponent
		Rectangle {
			width: 50
			height: 60
			color: "red"
		}
	}

	Component {
		id: connectionInfoComponent
		Page {
			ConnectionInfo {
				anchors.centerIn: parent
				remoteAddress: "127.0.0.1"
				remotePort: "13370"
				onCancelClicked: function() {
					console.log("onCancelClicked()");
				}
				onConnectClicked: function(remoteAddress, remotePort) {
					loader.sourceComponent = conferenceComponent
					app.connectToServer(remoteAddress, remotePort)
				}
			}
		}
	}

	Component {
		id: conferenceComponent
		Conference {
			onOpenDrawer: drawer.open()
		}
	}
}
