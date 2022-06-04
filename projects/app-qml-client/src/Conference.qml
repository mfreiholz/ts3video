import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 5.15

Page {
	id: root
	title: "Conference"

	signal openDrawer()

	header: ToolBar {
		RowLayout {
			anchors.fill: parent
			ToolButton {
				text: qsTr("Menu")
				onClicked: root.openDrawer()
			}
		}
	}

	footer: ToolBar {
		RowLayout {
			anchors.fill: parent
			ToolButton {
				text: qsTr("Camera")
				checkable: true
				checked: app.cameraVideoAdapter.cameraEnabled
				onToggled: {
					app.cameraVideoAdapter.cameraEnabled = checked
				}
			}
			ToolButton {
				text: "X"
				onClicked: cameraMenu.open()
				Menu {
					id: cameraMenu
					bottomMargin: parent.height
					MenuItem { text: "Item #1" }
					MenuItem { text: "Item #2" }
					MenuItem { text: "Item #3" }
					MenuSeparator {}
					Repeater {
						model: QtMultimedia.availableCameras
						MenuItem {
							text: modelData.displayName
							font.bold: modelData.deviceId == app.cameraVideoAdapter.deviceName
							onClicked: {
								console.debug(modelData.deviceId)
							}
						}
					}
				}
			}
			Item { Layout.fillWidth: true; }
			Label {
				text: qsTr("Clients: " + app.userInfoList.count)
			}
		}
	}

	GridLayout {
		id: grid
		anchors.fill: parent
		anchors.margins: 5
		columns: 3

		Frame {
			Layout.fillWidth: true
			Layout.fillHeight: true
			CameraVideo {
				anchors.fill: parent
			}
		}

		Repeater {
			id: remoteVideoRepeater
			model: app.userInfoList

			Frame {
				Layout.fillWidth: true
				Layout.fillHeight: true
				RemoteVideo {
					anchors.fill: parent
					remoteClientId: clientId
				}
			}
		}
	}

	/*********************************************************************
		Logic
	*********************************************************************/

	Component.onCompleted: {
		// Setup camera.
		let cam = QtMultimedia.defaultCamera
		if (cam)
			app.cameraVideoAdapter.deviceName = cam.deviceId
	}

	Component.onDestruction: {
	}

	onWidthChanged: {
		calcGrid();
	}

	onHeightChanged: {
		calcGrid();
	}

	function calcGrid() {
//		if (videoFramesModel.count <= 0)
//			return;
//		let item = videoFramesModel.itemAt(0)
//		if (item.width == 0)
//			return;
	}

	Connections {
		target: app.userInfoList

		function onClientAdded(clientId) {
			console.debug("Enter onClientAdded("+clientId+")")
			console.debug("Leave onClientAdded("+clientId+")")
		}

		function onClientRemoved(clientId) {
			console.debug("Enter onClientRemoved("+clientId+")")
			console.debug("Leave onClientRemoved("+clientId+")")
		}
	}
}
