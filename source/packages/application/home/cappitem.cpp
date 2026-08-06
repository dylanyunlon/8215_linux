/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
 
#include "cappitem.h"
#include "cglobaldata.h"

CAppItem::CAppItem(QObject *parent) :
    QObject(parent)
    ,m_id(0)
    ,m_page(0)
    ,m_index(0)
    ,m_xpos(0)
    ,m_ypos(0)
    ,m_width(0)
    ,m_height(0)
    ,m_textEn("")
    ,m_textZh("")
    ,m_textTw("")
    ,m_normalPicPath("")
    ,m_text("")
    ,m_rootObject(NULL)  
    ,m_visible(false)
    ,m_enable(false)
    ,m_normalTextColor(DEFAULT_NORMAL_TEXT_COLOR)
    ,m_disableTextColor(DEFAULT_DISABLE_TEXT_COLOR)
{

}
//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::CAppItem
//     
//  @Param : 
//        - int id
//        - int page
//        - int index
//        - int xpos
//        - int ypos
//        - int width
//        - int height
//        - QString textEn
//        - QString textZh
//        - QString textTw
//        - QString normalPicPath
//        - QString text
//     
//  @Return : None
//     
//  @Description :Init Object Model AppItem
//     
//---------------------------------------------------------------------------------
CAppItem::CAppItem(int id,
         int page, int index,
         int xpos, int ypos, int width, int height,
         QString textEn, QString textZh, QString textTw,
         QString normalPicPath, QString text, bool visible,
         bool enable, QString normalTextColor, QString disableTextColor)
    :m_id(id)
    ,m_page(page)
    ,m_index(index)
    ,m_xpos(xpos)
    ,m_ypos(ypos)
    ,m_width(width)
    ,m_height(height)
    ,m_textEn(textEn)
    ,m_textZh(textZh)
    ,m_textTw(textTw)
    ,m_normalPicPath(normalPicPath)
    ,m_text(text)
    ,m_visible(visible)
    ,m_enable(enable)
    ,m_normalTextColor(normalTextColor)
    ,m_disableTextColor(disableTextColor)
{    
}

CAppItem::~CAppItem()
{

}


//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::initAppItem
//     
//  @Param : - QObject
//     
//  @Return : void
//     
//  @Description :Init AppItem from QObject
//     
//---------------------------------------------------------------------------------
void CAppItem::initAppItem(QObject *rootObject)
{
    m_rootObject = rootObject;
}


//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::toString
//     
//  @Param : - None
//     
//  @Return : BOOL
//     
//  @Description :print params of object model AppItem
//     
//---------------------------------------------------------------------------------
bool CAppItem::toString()
{
    LOGI(TAG_HOME, 
        "appItem:[%d, %d, %d, %d, %d, %d, %d, %s, %s, %s, %s, %s]",
        m_id, 
        m_page, 
        m_index, 
        m_xpos, 
        m_ypos, 
        m_width, 
        m_height,
        (m_textEn.toLatin1()).data(),
        (m_textZh.toLatin1()).data(),
        (m_textTw.toLatin1()).data(),
        (m_normalPicPath.toLatin1()).data(),
        (m_text.toLatin1()).data());
    return true;
}


//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setId
//     
//  @Param : 
//        - int id
//     
//  @Return : void
//     
//  @Description :set AppItem id
//     
//---------------------------------------------------------------------------------
void CAppItem::setId(const int id)
{
    m_id = id;
    emit onIdChange(id);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setPage
//     
//  @Param : 
//        - int page
//     
//  @Return : void
//     
//  @Description :set AppItem page
//     
//---------------------------------------------------------------------------------
void CAppItem::setPage(const int page)
{
    m_page = page;
    emit onPageChange(page);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setIndex
//     
//  @Param : 
//        - int index
//     
//  @Return : void
//     
//  @Description :set AppItem index
//     
//---------------------------------------------------------------------------------
void CAppItem::setIndex(const int index)
{
    m_index = index;
    emit onIndexChange(index);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setXpos
//     
//  @Param : 
//        - int xpos
//     
//  @Return : void
//     
//  @Description :set AppItem xpos
//     
//---------------------------------------------------------------------------------
void CAppItem::setXpos(const int xpos)
{
    m_xpos = xpos;
    emit onxPosChange(xpos);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setYpos
//     
//  @Param : 
//        - int ypos
//     
//  @Return : void
//     
//  @Description :set AppItem ypos
//     
//---------------------------------------------------------------------------------
void CAppItem::setYpos(const int ypos)
{
    m_ypos = ypos;
    emit onyPosChange(ypos);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setWidth
//     
//  @Param : 
//        - int width
//     
//  @Return : void
//     
//  @Description :set AppItem width
//     
//---------------------------------------------------------------------------------
void CAppItem::setWidth(const int width)
{
    m_width = width;
    emit onWidthChange(width);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setHeight
//     
//  @Param : 
//        - int height
//     
//  @Return : void
//     
//  @Description :set AppItem height
//     
//---------------------------------------------------------------------------------
void CAppItem::setHeight(const int height)
{
    m_height = height;
    emit onHeightChange(height);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setTextEn
//     
//  @Param : 
//        - int text
//     
//  @Return : void
//     
//  @Description :set AppItem textEn
//     
//---------------------------------------------------------------------------------
void CAppItem::setTextEn(const QString text)
{
    m_textEn = text;
    emit onTextEnChange(text);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setTextZh
//     
//  @Param :
//        - int text
//     
//  @Return : void
//     
//  @Description :set AppItem textZh
//     
//---------------------------------------------------------------------------------
void CAppItem::setTextZh(const QString text)
{
    m_textZh = text;
    emit onTextZhChange(text);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setTextTw
//     
//  @Param :
//        - int text
//     
//  @Return : void
//     
//  @Description :set AppItem textTw
//     
//---------------------------------------------------------------------------------
void CAppItem::setTextTw(const QString text)
{
    m_textTw = text;
    emit onTextTwChange(text);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setNormalPicPath
//     
//  @Param :
//        - QString path
//     
//  @Return : void 
//     
//  @Description :set AppItem picture path
//     
//---------------------------------------------------------------------------------
void CAppItem::setNormalPicPath(const QString path)
{
    m_normalPicPath = path;
    emit onNormalPicChange(path);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::setText
//     
//  @Param :
//        - QString text
//     
//  @Return : void 
//     
//  @Description :set AppItem text
//     
//---------------------------------------------------------------------------------
void CAppItem::setText(const QString text)
{
    m_text = text;
    emit onTextChange(text);
}


void CAppItem::setVisible(bool visible)
{
    m_visible = visible;
}

void CAppItem::setEnable(bool enable)
{
    m_enable = enable;
}

void CAppItem::setNormalTextColor(QString textColor)
{
    m_normalTextColor = textColor;
}

void CAppItem::setDisableTextColor(QString textColor)
{
    m_disableTextColor = textColor;
}


//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onIdChange
//     
//  @Param :
//        - int id
//     
//  @Return : void
//     
//  @Description :Response on AppItem Id Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onIdChange(int id)
{
    LOGI(TAG_HOME, "CAppItem::onIdChange:%d\r\n",id);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onIdChange
//     
//  @Param :
//        - int id
//     
//  @Return : void
//     
//  @Description :Response on AppItem Id Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onPageChange(int page)
{
    LOGI(TAG_HOME, "CAppItem::onPageChange:%d\r\n",page);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onIdChange
//     
//  @Param :
//        - int id
//     
//  @Return : void
//     
//  @Description :Response on AppItem Id Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onIndexChange(int index)
{
    LOGI(TAG_HOME, "CAppItem::onIndexChange:%d\r\n",index);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onxPosChange
//     
//  @Param :
//        - int xpos
//     
//  @Return : void
//     
//  @Description :Response on AppItem xpos Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onxPosChange(int xpos)
{
    LOGI(TAG_HOME, "CAppItem::onxPosChange:%d\r\n",xpos);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onyPosChange
//     
//  @Param :
//        - int ypos
//     
//  @Return : void
//     
//  @Description :Response on AppItem ypos Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onyPosChange(int ypos)
{
    LOGI(TAG_HOME, "CAppItem::onyPosChange:%d\r\n",ypos);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onWidthChange
//     
//  @Param :
//        - int width
//     
//  @Return : void
//     
//  @Description :Response on AppItem width Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onWidthChange(int width)
{
    LOGI(TAG_HOME, "CAppItem::onWidthChange:%d\r\n",width);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onHeightChange
//     
//  @Param :
//        - int height
//     
//  @Return : void
//     
//  @Description :Response on AppItem height Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onHeightChange(int height)
{
    LOGI(TAG_HOME, "CAppItem::onHeightChange:%d\r\n",height);
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onTextEnChange
//     
//  @Param :
//        - QString textEn
//     
//  @Return : void
//     
//  @Description :Response on AppItem textEn Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onTextEnChange(QString textEn)
{
    LOGI(TAG_HOME, "CAppItem::onTextEnChange:%s\r\n", (textEn.toLatin1()).data());
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onTextZhChange
//     
//  @Param :
//        - QString textZh
//     
//  @Return : void
//     
//  @Description :Response on AppItem textZh Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onTextZhChange(QString textZh)
{
    LOGI(TAG_HOME, "CAppItem::onTextZhChange:%s\r\n", (textZh.toLatin1()).data());
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onTextTwChange
//     
//  @Param :
//        - QString textTw
//     
//  @Return : void
//     
//  @Description :Response on AppItem textTw Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onTextTwChange(QString textTw)
{
    LOGI(TAG_HOME, "CAppItem::onTextTwChange:%s\r\n", (textTw.toLatin1()).data());
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onNormalPicChange
//     
//  @Param :
//        - QString normalPicpath
//     
//  @Return : void
//     
//  @Description :Response on AppItem picture path Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onNormalPicChange(QString normalPicpath)
{
    LOGI(TAG_HOME, "CAppItem::onNormalPicChange:%s\r\n", (normalPicpath.toLatin1()).data());
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::onTextChange
//     
//  @Param :
//        - QString text
//     
//  @Return : void
//     
//  @Description :Response on AppItem text Changed
//     
//---------------------------------------------------------------------------------
void CAppItem::onTextChange(QString text)
{
    LOGI(TAG_HOME, "CAppItem::onTextChange: %s\r\n", (text.toLatin1()).data());
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::id
//     
//  @Param : - None
//     
//  @Return : int
//     
//  @Description :get AppItem id
//     
//---------------------------------------------------------------------------------
int CAppItem::getId(void)const
{
    return m_id;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::page
//     
//  @Param : - None
//     
//  @Return : int
//     
//  @Description :get AppItem page
//     
//---------------------------------------------------------------------------------
int CAppItem::page(void)const
{
    return m_page;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::index
//     
//  @Param : - None
//     
//  @Return : int
//     
//  @Description :get AppItem index
//     
//---------------------------------------------------------------------------------
int CAppItem::index(void)const
{
    return m_index;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::xpos
//     
//  @Param : - None
//     
//  @Return : int
//     
//  @Description :get AppItem xpos
//     
//---------------------------------------------------------------------------------
int CAppItem::xpos(void)const
{
    return m_xpos;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::ypos
//     
//  @Param : - None
//     
//  @Return : int
//     
//  @Description :get AppItem ypos
//     
//---------------------------------------------------------------------------------
int CAppItem::ypos(void)const
{
    return m_ypos;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::width
//     
//  @Param : - None
//     
//  @Return : int
//     
//  @Description :get AppItem width
//     
//---------------------------------------------------------------------------------
int CAppItem::width(void)const
{
    return m_width;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::height
//     
//  @Param : - None
//     
//  @Return : int
//     
//  @Description :get AppItem height
//     
//---------------------------------------------------------------------------------
int CAppItem::height(void)const
{
    return m_height;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::getTextEn
//     
//  @Param : - None
//     
//  @Return : QString
//     
//  @Description :get AppItem textEn
//     
//---------------------------------------------------------------------------------
QString CAppItem:: getTextEn(void)const
{
    return m_textEn;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::getTextZh
//     
//  @Param : - None
//     
//  @Return : QString
//     
//  @Description :get AppItem textZh
//     
//---------------------------------------------------------------------------------
QString CAppItem:: getTextZh(void)const
{
    return m_textZh;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::getTextTw
//     
//  @Param : - None
//     
//  @Return : QString
//     
//  @Description :get AppItem textTw
//     
//---------------------------------------------------------------------------------
QString CAppItem:: getTextTw(void)const
{
    return m_textTw;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::getNormalPicPath
//     
//  @Param : - None
//     
//  @Return : QString
//     
//  @Description :get AppItem picture path
//     
//---------------------------------------------------------------------------------
QString CAppItem:: getNormalPicPath(void)const
{
    return m_normalPicPath;
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItem::getText
//     
//  @Param : - None
//     
//  @Return : QString
//     
//  @Description :get AppItem text
//     
//---------------------------------------------------------------------------------
QString CAppItem:: getText(void)const
{
    return m_text;
}


bool CAppItem:: isVisible(void) const
{
    return m_visible;
}

bool CAppItem:: isEnable(void) const
{
    return m_enable;
}

QString CAppItem:: getNormalTextColor(void) const
{
    return m_normalTextColor;
}

QString CAppItem:: getDisableTextColor(void) const
{
    return m_disableTextColor;
}
