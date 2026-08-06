import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Item
{
	property alias value: slider.value
	property alias maximumValue: slider.maximumValue
	property alias text: title.text
	

    Text {
    	id: title
    	color: "white"
    	font.pixelSize: 35
    	elide: Text.ElideRight
    	anchors.top:parent.top
    	anchors.topMargin: 15
    	width: parent.width
    	horizontalAlignment: Text.AlignHCenter
    	anchors.horizontalCenter: parent.horizontalCenter          
	}
	            
	Slider {
	    id: slider
	    anchors.top: title.bottom
		anchors.topMargin: 15
		anchors.left:parent.left
		anchors.right:parent.right
		anchors.leftMargin: 30
		anchors.rightMargin: 60
		
	    style: SliderStyle {
	        groove:Rectangle{
	            implicitHeight: 4;
	            implicitWidth: slider.width;
	            color: "#FFFFFF";
	            radius: 8;
	            Rectangle {
	                antialiasing: true
	                radius: 8
	                color: "Yellow"
	                height: parent.height
	                width: parent.width * slider.value / slider.maximumValue
	            }
	        }
	        //slider block
	        handle: Rectangle{
	            width: 20
	            height: 20
	            radius: 20
	            antialiasing: true
	            color: control.pressed ? "Yellow":"#ffffff";
	        }
	    }
	}

    Label {
        id: volumeValue
        anchors.left: slider.right
        anchors.leftMargin: 10
		anchors.rightMargin: 30
        //height: 30
        color: "#ffffff"
        text: slider.value
        anchors.verticalCenter: slider.verticalCenter
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        styleColor: "#fbfbfb"
        font.pixelSize: 30
    }
}


