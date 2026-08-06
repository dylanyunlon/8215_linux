import QtQuick 2.2
import QtQuick.Controls 1.2

Item {
    id: fixedArea
    objectName: "fixedPageName"
    x: 0
    y: 411
    width: 1080
    height: 190
    visible: true
		
    property int fixed_page_language: 11
    
    Connections {
        target: csubwndhome
        onSendMsgToQml: {            
            switch (msg) {
                case fixed_page_language:
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
        var count = Page0Count;
        
        var itemIndex = 1;
        var item;
                  
            while(itemIndex <= count)
            {
                switch(itemIndex){
                case 1:
                    item = Page0AppItem1;
                    setItemLanguage(item, languageIndex);
                    break;
                case 2:
                    item = Page0AppItem2;
                    setItemLanguage(item, languageIndex);
                    break;
                case 3:
                    item = Page0AppItem3;
                    setItemLanguage(item, languageIndex);
                    break;
                case 4:
                    item = Page0AppItem4;
                    setItemLanguage(item, languageIndex);
                    break;
                case 5:
                    item = Page0AppItem5;
                    setItemLanguage(item, languageIndex);
                    break;
                case 6:
                    item = Page0AppItem6;
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

    Component.onCompleted: {
        var count = Page0Count;
        var startNum = Page0Start;
        console.log("FixedPage:",count);
        console.log("FixedPageStart", startNum);
        var index = 1;  //用来循环自动生成的Item
        var itemIndex = 0;
        var item;
        while(index <= count)
        {
            switch(index){
            case 1:
                item = Page0AppItem1;
                itemIndex = Page0AppItem1.index;
                console.log("1item,id:",item.appid);
                console.log("1itemIndex:",itemIndex);
                setItemIndex(item, itemIndex);
                break;
            case 2:
                item = Page0AppItem2;
                itemIndex = Page0AppItem2.index;
                console.log("1item,id:",item.appid);
                console.log("1itemIndex:",itemIndex);
                setItemIndex(item, itemIndex);
                break;
            case 3:
                item = Page0AppItem3;
                itemIndex = Page0AppItem3.index;
                console.log("1item,id:",item.appid);
                console.log("1itemIndex:",itemIndex);
                setItemIndex(item, itemIndex);
                break;
            case 4:
                item = Page0AppItem4;
                itemIndex = Page0AppItem4.index;
                console.log("1item,id:",item.appid);
                console.log("1itemIndex:",itemIndex);
                setItemIndex(item, itemIndex);
                break;
            case 5:
                item = Page0AppItem5;
                itemIndex = Page0AppItem5.index;
                console.log("1item,id:",item.appid);
                console.log("1itemIndex:",itemIndex);
                setItemIndex(item, itemIndex);
                break;
            case 6:
                item = Page0AppItem6;
                itemIndex = Page0AppItem6.index;
                console.log("1item,id:",item.appid);
                console.log("1itemIndex:",itemIndex);
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
