import QtQuick 2.2
import QtQuick.Controls 1.2

Item{
    objectName: "appItemPageName"
    visible:true

    property var itemId
    property alias itemXPos:appImage.x
    property alias itemYPos:appImage.y
    property alias itemWidth:appImage.width
    property alias itemHeight:appImage.height
    property alias itemPicPath:appImage.source
    property alias itemText:appText.text
    property alias itemTextColor:appText.color

    Image {
        id:appImage
        visible: true
        MouseArea{
            anchors.fill: parent

            Image{
                id:shadeImage
                visible: false
                opacity: 0.3
                anchors.fill: parent
                source:"3rdparty2.png"
            }
            onPressed: {
                anim.running = true;
            }
            onClicked: {
                anim.running = true;
                appItemClicked(itemId);
            }
        }
    }
    SequentialAnimation
    {
        id:anim

        NumberAnimation {
            target:appImage
            property:"opacity"
            from : 1.0
            to : 0.3
            duration:100
        }
        NumberAnimation
        {
            target:appImage
            property:"opacity"
            from : 0.3
            to : 1.0
            duration:700
        }
    }

    Text{
        id:appText
        visible:true
        height: 50
        color:"white"
        anchors.top: appImage.bottom
        anchors.horizontalCenter: appImage.horizontalCenter
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font{bold: true; pixelSize: 18}
    }
}
