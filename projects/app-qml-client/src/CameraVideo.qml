import QtQuick 2.15
import QtMultimedia 5.15

Item {
	id: root

	VideoOutput {
		id: videoOutput
		anchors.fill: parent
		fillMode: VideoOutput.PreserveAspectFit
		source: app.cameraVideoAdapter
	}
}
