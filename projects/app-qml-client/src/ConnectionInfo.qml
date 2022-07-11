import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Frame {
	id: root
	width: 400
	height: 200
	property alias remoteAddress: remoteAddressInput.text
	property alias remotePort: remotePortInput.text
	signal cancelClicked()
	signal connectClicked(string remoteAddress, int remotePort)

	ColumnLayout {
		anchors.fill: parent

		Label {
			text: "Server address"
		}

		TextField {
			Layout.fillWidth: true
			id: remoteAddressInput
		}

		Label {
			text: "Server port"
		}

		TextField {
			Layout.fillWidth: true
			id: remotePortInput
		}

		RowLayout {
			Layout.fillWidth: true

			Item {
				Layout.fillWidth: true
			}
			Button {
				text: "Cancel"
				onClicked: root.cancelClicked()
			}
			Button {
				text: "Connect"
				highlighted: true
				onClicked: root.connectClicked(remoteAddressInput.text, remotePortInput.text)
			}
		}
	}
}
