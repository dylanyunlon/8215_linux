import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQml.Models 2.1

Item {
    id: scrollPage
    x:0
    y:41
    width: 1080
    height: 380

    property int curScrollPage : 1;
    property int pages: ScrollPages;


    function gotoScrollPage(index){
        view.currentIndex = index
    }

    Image {
        id: imageHome
        x: 30
        y: -25
        width: 41
        height: 23
        source: "home.png"
        Image{
            id:shadeImageHome
            anchors.fill: parent
            visible: false
            opacity: 0.3
            source:"3rdparty2.png"
        }
        MouseArea{
            anchors.fill: parent
            onPressed: {
                gotoScrollPage(0);
                shadeImageHome.visible = true;
            }
            onReleased: {
                shadeImageHome.visible = false;
            }
        }
    }

    Image {
        id: imageBack
        anchors.right: parent.right
        y:-23
        width: 35
        height: 20
        anchors.leftMargin: 20
        anchors.rightMargin: 15
        source: "back.png"
        Image{
            id:shadeImageBack
            anchors.fill: parent
            visible: false
            opacity: 0.3
            source:"3rdparty2.png"
        }
        MouseArea{
            anchors.fill: parent
            onPressed: {
                gotoScrollPage(0);
                shadeImageBack.visible = true;
            }
            onReleased: {
                shadeImageBack.visible = false;
            }
        }
    }


    ObjectModel {
        id: itemModel

        FirstPage{}
        //FollowingPage{}
       // Repeater{
           // model:pages
           // FollowingPage{}
       // }

    }

    ListView {
        id: view
        anchors { fill: parent; bottomMargin: 10 }
        model: itemModel
        preferredHighlightBegin: 0; preferredHighlightEnd: 0
        highlightRangeMode: ListView.StrictlyEnforceRange
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem; flickDeceleration: 200
        cacheBuffer: 200
        boundsBehavior: Flickable.StopAtBounds
        highlightMoveDuration:500
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width; height: 5
        anchors { top: view.bottom; bottom: parent.bottom }
        color: "#00ffffff"

        Row {
            anchors.centerIn: parent
            spacing: 20

            Repeater {
                model: itemModel.count

                Rectangle {
                    width: 15; height: 5
                    radius: 3
                    color: view.currentIndex == index ? "white" : "blue"

                    MouseArea {
                        anchors.fill: parent
                        anchors.centerIn: parent
                        //onClicked: view.currentIndex = index
                        onClicked:{
                            gotoScrollPage(index);
                        }
                    }
                }
            }
        }
    }

}
