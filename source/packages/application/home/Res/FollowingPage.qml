import QtQuick 2.0

Item {
    id: scrollPage2
    objectName: "followingPageName"
    visible: true
    width: 1080
    height: 360
     
    property int following_page_language: 11
    
    Connections {
        target: csubwndhome
        onSendMsgToQml: {            
            switch (msg) {
                case following_page_language:
                    updateText(wParam);
                    break;
                default:
                    break;
            }
        }
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
        case 7:
            appItemUI = appItemUI7;
            break;
        case 8:
            appItemUI = appItemUI8;
            break;
        case 9:
            appItemUI = appItemUI9;
            break;
        case 10:
            appItemUI = appItemUI10;
            break;
        case 11:
            appItemUI = appItemUI11;
            break;
        case 12:
            appItemUI = appItemUI12;
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
        var count = Page2Count;
        
        var itemIndex = 1;
        var item;
                  
            while(itemIndex <= count)
            {
                switch(itemIndex){
                case 1:
                    item = Page2AppItem1;
                    setItemLanguage(item, languageIndex);
                    break;
                case 2:
                    item = Page2AppItem2;
                    setItemLanguage(item, languageIndex);
                    break;
                case 3:
                    item = Page2AppItem3;
                    setItemLanguage(item, languageIndex);
                    break;
                case 4:
                    item = Page2AppItem4;
                    setItemLanguage(item, languageIndex);
                    break;
                case 5:
                    item = Page2AppItem5;
                    setItemLanguage(item, languageIndex);
                    break;
                case 6:
                    item = Page2AppItem6;
                    setItemLanguage(item, languageIndex);
                    break;
                case 7:
                    item = Page2AppItem7;
                    setItemLanguage(item, languageIndex);
                    break;
                case 8:
                    item = Page2AppItem8;
                    setItemLanguage(item, languageIndex);
                    break;
                case 9:
                    item = Page2AppItem9;
                    setItemLanguage(item, languageIndex);
                    break;
                case 10:
                    item = Page2AppItem10;
                    setItemLanguage(item, languageIndex);
                    break;
                case 11:
                    item = Page2AppItem11;
                    setItemLanguage(item, languageIndex);
                    break;
                case 12:
                    item = Page2AppItem12;
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
        case 7:
            appItemUI = appItemUI7;
            break;
        case 8:
            appItemUI = appItemUI8;
            break;
        case 9:
            appItemUI = appItemUI9;
            break;
        case 10:
            appItemUI = appItemUI10;
            break;
        case 11:
            appItemUI = appItemUI11;
            break;
        case 12:
            appItemUI = appItemUI12;
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

    Component.onCompleted: {
        var count = Page2Count;
        var startNum = Page2Start;
        console.log("ScrollPage:",count);
        console.log("FollowingPageStart", startNum);
        var index = 1;  //用来循环自动生成的Item
        var itemIndex = 0;
        var item;
        while(index <= count)
        {
            switch(index){
            case 1:
                item = Page2AppItem1;
                itemIndex = Page2AppItem1.index;
                setItemIndex(item, itemIndex);
                break;
            case 2:
                item = Page2AppItem2;
                itemIndex = Page2AppItem2.index;
                setItemIndex(item, itemIndex);
                break;
            case 3:
                item = Page2AppItem3;
                itemIndex = Page2AppItem3.index;
                setItemIndex(item, itemIndex);
                break;
            case 4:
                item = Page2AppItem4;
                itemIndex = Page2AppItem4.index;
                setItemIndex(item, itemIndex);
                break;
            case 5:
                item = Page2AppItem5;
                itemIndex = Page2AppItem5.index;
                setItemIndex(item, itemIndex);
                break;
            case 6:
                item = Page2AppItem6;
                itemIndex = Page2AppItem6.index;
                setItemIndex(item, itemIndex);
                break;
            case 7:
                item = Page2AppItem7;
                itemIndex = Page2AppItem7.index;
                setItemIndex(item, itemIndex);
                break;
            case 8:
                item = Page2AppItem8;
                itemIndex = Page2AppItem8.index;
                setItemIndex(item, itemIndex);
                break;
            case 9:
                item = Page2AppItem9;
                itemIndex = Page2AppItem9.index;
                setItemIndex(item, itemIndex);
                break;
            case 10:
                item = Page2AppItem10;
                itemIndex = Page2AppItem10.index;
                setItemIndex(item, itemIndex);
                break;
            case 11:
                item = Page2AppItem11;
                itemIndex = Page2AppItem11.index;
                setItemIndex(item, itemIndex);
                break;
            case 12:
                item = Page2AppItem12;
                itemIndex = Page2AppItem12.index;
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
        itemYPos:30
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
        itemYPos:30
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
        itemYPos:30
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
        itemYPos:30
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
        itemYPos:30
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
        itemYPos:30
        itemWidth:110
        itemHeight:110
        itemPicPath:""
        itemText:""
    }

    AppItem{
        id:appItemUI7
        visible:false
        enabled:false
        itemXPos:24
        itemYPos:200
        itemWidth:110
        itemHeight:110
        itemPicPath:""
        itemText:""
    }
    AppItem{
        id:appItemUI8
        visible:false
        enabled:false
        itemXPos:192
        itemYPos:200
        itemWidth:110
        itemHeight:110
        itemPicPath:""
        itemText:""
    }
    AppItem{
        id:appItemUI9
        visible:false
        enabled:false
        itemXPos:360
        itemYPos:200
        itemWidth:110
        itemHeight:110
        itemPicPath:""
        itemText:""
    }
    AppItem{
        id:appItemUI10
        visible:false
        enabled:false
        itemXPos:528
        itemYPos:200
        itemWidth:110
        itemHeight:110
        itemPicPath:""
        itemText:""
    }
    AppItem{
        id:appItemUI11
        visible:false
        enabled:false
        itemXPos:696
        itemYPos:200
        itemWidth:110
        itemHeight:110
        itemPicPath:""
        itemText:""
    }
    AppItem{
        id:appItemUI12
        visible:false
        enabled:false
        itemXPos:862
        itemYPos:200
        itemWidth:110
        itemHeight:110
        itemPicPath:""
        itemText:""
    }
}

