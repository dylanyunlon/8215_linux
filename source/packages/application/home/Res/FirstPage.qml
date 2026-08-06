import QtQuick 2.0

Item {
    id:scrollPage1
    objectName: "firstPageName"
    width: 1080
    height: 360
    
    property int first_page_language:   11
    property int first_page_timeformat: 12
    property int first_page_weather:    13
    
    property int first_page_language_id: 0
    
    property int time_format_12H:                   0
    property int time_format_24H:                   1
    property int cur_time_format:                   time_format_24H
    
    property int weather_city:                      0
    property int weather_weather:                   1
    property int weather_temp:                      2

    Connections {
        target: csubwndhome
        onSendMsgToQml: {            
            switch (msg) {
                case first_page_language:
                    updateText(wParam);
                    first_page_language_id=wParam;
                    console.log("[HOME_APP]FirstPage,update language:"+wParam);
                    break;
                case first_page_timeformat:
                    console.log("[HOME_APP]FirstPage,update timeformat:"+wParam);
                    switch(wParam)
                    {
                    case time_format_12H:
                        cur_time_format = time_format_12H;
                        break;
                    case time_format_24H:
                        cur_time_format = time_format_24H;
                        break;                   
                    default:
                        break;
                    }
                    updatelocaltime();
                    break;
                case first_page_weather:
                    console.log("[HOME_APP]FirstPage,update weather:"+wParam);
                    switch(wParam)
                    {
                    case weather_city:
                        console.log("[HOME_APP]FirstPage,update weather:city"+lParam);
                        textCity.text = lParam;
                        break;
                    case weather_weather:
                        console.log("[HOME_APP]FirstPage,update weather:weather"+lParam);
                        textWeather.text = lParam;
                        break;
                    case weather_temp:
                        console.log("[HOME_APP]FirstPage,update weather:temp"+lParam);
                        textTemp.text = lParam;
                        break;
                    }
                    break;                  
                default:
                    break;
            }
        }
    }
    
    function getWeekDay(curWeekDay)
    {
        var strCurWeekDay;
        if(first_page_language_id == 0)
        {
                if (curWeekDay == 1) {
                    strCurWeekDay =  "MON";
                }
                else if (curWeekDay == 2) {
                    strCurWeekDay = "TUE";
                }
                else if (curWeekDay == 3) {
                    strCurWeekDay =  "WED";
                }
                else if (curWeekDay == 4) {
                    strCurWeekDay =  "THU";
                }
                else if (curWeekDay == 5) {
                    strCurWeekDay =  "FRI";
                }
                else if (curWeekDay == 6) {
                    strCurWeekDay =  "SAT";
                } 
                else if (curWeekDay == 0) {
                    strCurWeekDay =  "SUN";
                }
        }
        else if(first_page_language_id == 1)   
        {     
                if (curWeekDay == 1) {
                    strCurWeekDay =  "\u661f\u671f\u4e00";
                }
                else if (curWeekDay == 2) {
                    strCurWeekDay =  "\u661f\u671f\u4e8c";
                }
                else if (curWeekDay == 3) {
                    strCurWeekDay =  "\u661f\u671f\u4e09";
                }
                else if (curWeekDay == 4) {
                    strCurWeekDay =  "\u661f\u671f\u56db";
                }
                else if (curWeekDay == 5) {
                    strCurWeekDay =  "\u661f\u671f\u4e94";
                }
                else if (curWeekDay == 6) {
                    strCurWeekDay =  "\u661f\u671f\u516d";
                }
                else if (curWeekDay == 0) {
                    strCurWeekDay =  "\u661f\u671f\u65e5";
                }
        }
        else if(first_page_language_id == 2)   
        {       
                if (curWeekDay == 1) {
                    strCurWeekDay =  "\u661f\u671f\u4e00";
                }
                else if (curWeekDay == 2) {
                    strCurWeekDay =  "\u661f\u671f\u4e8c";
                }
                else if (curWeekDay == 3) {
                    strCurWeekDay =  "\u661f\u671f\u4e09";
                }
                else if (curWeekDay == 4) {
                    strCurWeekDay =  "\u661f\u671f\u56db";
                }
                else if (curWeekDay == 5) {
                    strCurWeekDay =  "\u661f\u671f\u4e94";
                }
                else if (curWeekDay == 6) {
                    strCurWeekDay =  "\u661f\u671f\u516d";
                }
                else if (curWeekDay == 0) {
                    strCurWeekDay =  "\u661f\u671f\u65e5";
                }
        }
        return strCurWeekDay;
    }
    
    
        function updatelocaltime()
    {
        var d = new Date();        
        var tempHour = d.getHours();
        var tempMin = d.getMinutes();        
        var curWeekDay = d.getDay();
        var curYears = d.getFullYear();
        var curMonths = d.getMonth()+1;
        var curDay = d.getDate();

        var hour1=0;
        var hour2=0;
        var min1=0;
        var min2=0; 
        var curTime;
        var textDay = "";
        
        if (cur_time_format == time_format_12H) {
            if(tempHour >= 12)
            {
                if (tempHour > 12) {
                    tempHour -= 12;
                }
                textAMPM.text = "PM";
                if(first_page_language_id == 1 || first_page_language_id == 2)
                {
                    textAMPM.text = "\u4e0b\u5348";
                }
            } else {
                if (tempHour == 0) {
                    tempHour = 12;
                }
                textAMPM.text = "AM";
                if(first_page_language_id == 1 || first_page_language_id == 2)
                {
                    textAMPM.text = "\u4e0a\u5348";
                }
            }
        } else {
            textAMPM.text = "";
        }
        hour1 = parseInt(tempHour / 10);
        hour2 = tempHour % 10;
        min1 = parseInt(tempMin / 10);
        min2 = tempMin % 10;  
        hourH.source = getPngName(hour1);
        hourL.source = getPngName(hour2);
        dotdot.source="clockSlider.png"
        minH.source = getPngName(min1);
        minL.source = getPngName(min2);
            
        if(curWeekDay >= 0 && curWeekDay <= 6) {
            textDay = getWeekDay(curWeekDay) +"\n" + curYears + "-" + curMonths + "-" +curDay;
            textCurDay.text = textDay;
        }
    }
    
    Timer{
        interval: 1000;
        repeat: true
        running: true;
        triggeredOnStart: true
        onTriggered: {
            updatelocaltime();
        }
    }

    function getPngName(x)
    {
        return (x+".png");
    }
          
    function setItemLanguage(item, languageIndex)
    {
        var appItemUI;
        var itemUIIndex = item.index;
        var itemLanguageText;
        
        switch(itemUIIndex)
        {
        case 1:
            appItemUI = appItemUI1;
            break;
        case 2:
            appItemUI = appItemUI2;
            break;
        case 3:
            appItemUI = appItemUI3;
            break;
        case 4:
            appItemUI = appItemUI4;
            break;
        case 5:
            appItemUI = appItemUI5;
            break;
        case 6:
            appItemUI = appItemUI6;
            break;
        default:
            break;
        }
        
        if (languageIndex == 0) { 
            itemLanguageText = item.textEn;
        } else if (languageIndex == 1) {
            itemLanguageText = item.textZh;
        } else if (languageIndex == 2) {
            itemLanguageText = item.textTw;
        } else {
            itemLanguageText = "";
        }  
        
        appItemUI.itemText = itemLanguageText;
    }        
    
    function updateText(languageIndex)
    {
        var count = Page1Count;
        
        var itemIndex = 1;
        var item;
                  
            while(itemIndex <= count)
            {
                switch(itemIndex){
                case 1:
                    item = Page1AppItem1;
                    setItemLanguage(item, languageIndex);
                    break;
                case 2:
                    item = Page1AppItem2;
                    setItemLanguage(item, languageIndex);
                    break;
                case 3:
                    item = Page1AppItem3;
                    setItemLanguage(item, languageIndex);
                    break;
                case 4:
                    item = Page1AppItem4;
                    setItemLanguage(item, languageIndex);
                    break;
                case 5:
                    item = Page1AppItem5;
                    setItemLanguage(item, languageIndex);
                    break;
                case 6:
                    item = Page1AppItem6;
                    setItemLanguage(item, languageIndex);
                    break;
                default:
                    break;
                }
                
                itemIndex ++;
            }
    }

    function setItemIndex(item, itemIndex)
    {
          var appItemUI;
          
        switch(itemIndex)
        {
        case 1:
            appItemUI = appItemUI1;
            break;
        case 2:
            appItemUI = appItemUI2;
            break;
        case 3:
            appItemUI = appItemUI3;
            break;
        case 4:
            appItemUI = appItemUI4;
            break;
        case 5:
            appItemUI = appItemUI5;
            break;
        case 6:
            appItemUI = appItemUI6;
            break;
        default:
            return;
        }
        
        appItemUI.visible = item.visible;
        appItemUI.enabled = item.enable;
        appItemUI.itemId = item.appid;
        appItemUI.itemTextColor = item.enable? item.normalTextColor : item.disableTextColor;
        appItemUI.itemPicPath = item.normalPicpath;
    }
    
    Rectangle {
        id: rectangleTimeWeather
        color: "#00ffffff"

        Image{
            id:rectangleTimeWeatherImage
            x: 100
            y: 40
            width: 391
            height: 130
            opacity: 0.1
            source: "clock_background_30%.png"
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    timezoneClicked();
                }
            }
        }

        Image {
            id: hourH
            x: 115
            y: 65
            width: 35
            height: 70
        }
        Image {
            id: hourL
            x: 151
            y: 65
            width: 35
            height: 70
        }
        Image {
            id: dotdot
            x: 187
            y: 65
            width: 35
            height: 70
        }
        Image {
            id: minH
            x: 208
            y: 65
            width: 35
            height: 70
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
        Image {
            id: minL
            x: 247
            y: 65
            width: 35
            height: 70
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            id: textAMPM
            x: 293
            y: 65
            width: 50
            height: 30
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 20
        }

        Text {
            id: textCurDay
            x: 338
            y: 55
            width: 154
            height: 100
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 24
        }

        Image {
            id: rectangleWeather
            x: 653
            y: 40
            width: 279
            height: 130
            opacity: 0.1
            source: "clock_background_30%.png"
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    weatherzoneClicked(1);
                }
            }
        }

        Image {
            id: borderImage1
            x: 588
            y: 40
            width: 135
            height: 130
            source: "weather_cloudy.png"
        }

        Text {
            id: textCity
            x: 730
            y: 50
            width: 184
            height: 40
            color: "#ffffff"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 30
        }

        Text {
            id: textWeather
            x: 733
            y: 93
            width: 180
            height: 35
            color: "#ffffff"
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 24
        }
        Text {
            id: textTemp
            x: 733
            y: 128
            width: 180
            height: 35
            color: "#ffffff"
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 24
        }
    }
    
    Rectangle {
        id: scrollPage1Line2
        visible: true
        x: 0
        y: 181
        color: "#00ffffff"
        Component.onCompleted: {
            var count = Page1Count;
            var startNum = Page1Start;
            console.log("FirstPage:",count);
            console.log("FirstPageStart", startNum);

            var index = 1;
            var itemIndex = 0;
            var item;
            while(index <= count)
            {
                switch(index){
                case 1:
                    item = Page1AppItem1;
                    itemIndex = Page1AppItem1.index;
                    setItemIndex(item, itemIndex);
                    break;
                case 2:
                    item = Page1AppItem2;
                    itemIndex = Page1AppItem2.index;
                    setItemIndex(item, itemIndex);
                    break;
                case 3:
                    item = Page1AppItem3;
                    itemIndex = Page1AppItem3.index;
                    setItemIndex(item, itemIndex);
                    break;
                case 4:
                    item = Page1AppItem4;
                    itemIndex = Page1AppItem4.index;
                    setItemIndex(item, itemIndex);
                    break;
                case 5:
                    item = Page1AppItem5;
                    itemIndex = Page1AppItem5.index;
                    setItemIndex(item, itemIndex);
                    break;
                case 6:
                    item = Page1AppItem6;
                    itemIndex = Page1AppItem6.index;
                    setItemIndex(item, itemIndex);
                    break;
                default:
                    break;
                }
                index ++;
            }
        }

        AppItem{
            id:appItemUI1
            visible:false
            enabled:false
            itemXPos:24
            itemYPos:20
            itemWidth:110
            itemHeight:110
            itemPicPath:""
            itemText:""
        }
        AppItem{
            id:appItemUI2
            visible:false
            enabled:false
            itemXPos:192
            itemYPos:20
            itemWidth:110
            itemHeight:110
            itemPicPath:""
            itemText:""
        }
        AppItem{
            id:appItemUI3
            visible:false
            enabled:false
            itemXPos:360
            itemYPos:20
            itemWidth:110
            itemHeight:110
            itemPicPath:""
            itemText:""
        }
        AppItem{
            id:appItemUI4
            visible:false
            enabled:false
            itemXPos:528
            itemYPos:20
            itemWidth:110
            itemHeight:110
            itemPicPath:""
            itemText:""
        }
        AppItem{
            id:appItemUI5
            visible:false
            enabled:false
            itemXPos:696
            itemYPos:20
            itemWidth:110
            itemHeight:110
            itemPicPath:""
            itemText:""
        }
        AppItem{
            id:appItemUI6
            visible:false
            enabled:false
            itemXPos:862
            itemYPos:20
            itemWidth:110
            itemHeight:110
            itemPicPath:""
            itemText:""
        }

    }
}

