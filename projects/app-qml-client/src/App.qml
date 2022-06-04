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
		app.connectToServer()
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
		anchors.fill: parent
		sourceComponent: conferenceComponent
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
		id: conferenceComponent
		Conference {
			onOpenDrawer: drawer.open()
		}
	}
}
