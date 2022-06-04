import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 5.15

Pane {
	ColumnLayout {
		anchors.fill: parent

		Label {
			Layout.fillWidth: true
			text: "Camera"
		}
		ComboBox {
			Layout.fillWidth: true
			id: cameraComboBox
			textRole: "displayName"
			valueRole: "deviceId"
			// List of all available cameras.
			model: QtMultimedia.availableCameras
			// Set initial value.
			Component.onCompleted: currentIndex = indexOfValue(app.cameraVideoAdapter.deviceName)
		}
		Item {
			Layout.fillHeight: true
			Layout.fillWidth: true
		}
	}
}
