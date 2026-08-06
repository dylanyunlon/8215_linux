import QtQuick 2.2
import QtQml.Models 2.1
import QtQuick.Controls 1.2
import QtQuick.Window 2.2

Window {
    id:root
    objectName: "subwindowhome"
    visible: false
    width:1080
    height:600

    function appItemClicked(appID)
    {
        csubwndhome.appItemClicked(appID);
    }

    function timezoneClicked()
    {
        csubwndhome.timezoneClicked();
    }

    function weatherzoneClicked()
    {
        csubwndhome.weatherzoneClicked(2);
    }

    function topPageLoaded()
    {
        console.log("topPageLoaded from home.qml\n");
        csubwndhome.topPageLoaded();
    }

    function scrollPageLoaded()
    {
        console.log("scrollPageLoaded from home.qml\n");
        fixedPageLoader.visible=true;
        scrollPageLoader.visible=true;
        topbarLoader.visible=true;
        csubwndhome.scrollPageLoaded();
    }

    Rectangle{
        anchors.fill: parent
        Image{
            //source:"image://AppImageProvider/background.png"
            source: "bg.png"
            anchors.fill: parent
        }
    }

   //the following is using async loader
   Loader {
        id: topbarLoader
        asynchronous: false
        width: parent.width
        height: 56
        visible: false
        onStatusChanged:if(topbarLoader.status == Loader.Ready)
        {
            console.log("topbarLoader loaded!");
            topPageLoaded();
        }
    }

    Loader {
        id: scrollPageLoader
        asynchronous: false
        width: parent.width
        height: 380
        visible: false
        onStatusChanged:if(scrollPageLoader.status == Loader.Ready)
        {
           console.log("scrollPageLoader loaded!");
           scrollPageLoaded();
        }
    }

    Loader {
        id: fixedPageLoader
        asynchronous: false
        width: parent.width
        height: 190
        visible: false
        onStatusChanged:if(fixedPageLoader.status == Loader.Ready)
        {
            console.log("fixedPageLoader loaded!");
        }
    }

    Component.onCompleted: {
        console.log("[homeqml] Component.onCompleted is called\n");
        topbarLoader.source = "TopPage.qml"
        scrollPageLoader.source = "ScrollPage.qml"
        fixedPageLoader.source = "FixedPage.qml"
        console.log("[homeqml] Component.onCompleted is called done\n");
    }
}
