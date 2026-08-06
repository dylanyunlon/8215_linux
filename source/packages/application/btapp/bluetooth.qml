import QtQuick 2.5
import QtQuick.Window 2.2

Window {
    id:bluetoothapplication
    width: 1024
    height: 600

    function add_zero(temp)
    {
        if (temp - 0.5 >= 0.0000001) {
            temp -= 0.5;
        }
        temp = temp.toFixed(0);
        if(temp < 10)
            return "0"+temp;
        else
            return temp;
    }

    Rectangle {
        width: parent.width
    	height: parent.height
    	color:"#087b7f"
    }

    Loader {
		id: bluetoothTopbarLoader
		asynchronous: true
		width: parent.width
    	height: 44
    	visible: true
	}

	Loader {
		id: bluetoothPageLoader
		asynchronous: true
		anchors.top: bluetoothTopbarLoader.bottom
		width: parent.width
		height: parent.height - bluetoothTopbarLoader.height
		visible: true
	}

    // Connections to forward signals from bluetoothapplicationpage to bluetoothtopbar
    Connections {
        target: bluetoothPageLoader.item
        onSigHighlightTopbarButton: {
            if (bluetoothTopbarLoader.item) {
                bluetoothTopbarLoader.item.setHighlight(buttonIndex);
            }
        }
        onSigClearTopbarHighlight: {
            if (bluetoothTopbarLoader.item) {
                bluetoothTopbarLoader.item.clearHighlight();
            }
        }
    }

	Component.onCompleted: {
		console.log("[Bluetooth] Component.onCompleted is called\n");
		bluetoothTopbarLoader.source = "qml/bluetoothtopbar.qml"
		bluetoothPageLoader.source = "qml/bluetoothapplicationpage.qml"
	}

    Rectangle {
        anchors.top: bluetoothTopbarLoader.bottom
		width: parent.width
		height: parent.height - bluetoothTopbarLoader.height
        id: phonebookOverlay
        color: "#00000000"
        visible: false
        MouseArea {
            anchors.fill: parent
        }
    }

}