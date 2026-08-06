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

#include "cappitemparser.h"
#include "cglobaldata.h"
#include <QXmlStreamReader>
#include <QFile>
#include <QList>
#include <QMessageBox>
#include <string>
#include "appobj.h"

CAppItemParser::CAppItemParser(QObject *parent) :
    QObject(parent)
    ,m_rootObject(NULL)
{
}

CAppItemParser::~CAppItemParser()
{
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItemParser::initAppItemList
//
//  @Param :
//        - QObject *rootObject
//
//  @Return : void
//
//  @Description :init AppItem List
//
//---------------------------------------------------------------------------------
void CAppItemParser::initAppItemList(QObject *rootObject)
{
    m_rootObject = rootObject;
}

//--------------------------------------------------------------------------------
//  @Function Name : int
//
//  @Param :
//        - QString file_name
//
//  @Return : BOOL
//
//  @Description :parse xml from file_name
//
//---------------------------------------------------------------------------------
bool CAppItemParser::parseXML(QString file_name)
{
    LOGI(TAG_HOME, "file_name= %s \r\n",(file_name.toLatin1()).data());

    if(file_name.isEmpty())
    {
        LOGI(TAG_HOME, "file_name.isEmpty");
        return false;
    }

    QFile *file = new QFile(file_name);

    if(file == NULL)
    {
    LOGI(TAG_HOME, "new QFile error!");
        return false;
    }

    if(!file->open(QFile::ReadOnly | QFile::Text))
    {
        LOGI(TAG_HOME, "open %s error!", (file_name.toLatin1()).data());
        return false;
    }

    QXmlStreamReader xml(file);
    CAppItem *appItem;
    appItem = new CAppItem();

    int appItemId = 0;
    int appItemPage = 0;
    int appItemIndex = 0;
    int appItemXpos = 0;
    int appItemYpos = 0;
    int appItemWidth = 0;
    int appItemHeight = 0;
    bool visible = false;
    bool enable = false;
    QString normalTextColor = DEFAULT_NORMAL_TEXT_COLOR;
    QString disableTextColor = DEFAULT_DISABLE_TEXT_COLOR;
    QString appItemTextEn = "";
    QString appItemTextZh = "";
    QString appItemTextTw = "";
    QString appItemNormalPicPath = "";
    QString appItemText = "";
    QString tempStr = "";

    while(!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();

        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }

        if(token == QXmlStreamReader::StartElement)
        {
            if(xml.name() == "appitems")
            {
                continue;
            }

            if(xml.name() == "appitem")
            {
                QXmlStreamAttributes attributes = xml.attributes();
                if(attributes.hasAttribute("id"))
                {
                    appItemId = attributes.value("id").toInt();
                }

                xml.readNext();

                visible = true;
                enable = true;
                normalTextColor = DEFAULT_NORMAL_TEXT_COLOR;
                disableTextColor = DEFAULT_DISABLE_TEXT_COLOR;

                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "appitem"))
                {
                    if(xml.tokenType() == QXmlStreamReader::StartElement)
                    {
                        if(xml.name() == "page")
                        {
                            appItemPage = xml.readElementText().toInt();
                        }
                        if(xml.name() == "index")
                        {
                            appItemIndex = xml.readElementText().toInt();
                        }
                        if(xml.name() == "xpos")
                        {
                            appItemXpos = xml.readElementText().toInt();
                        }
                        if(xml.name() == "ypos")
                        {
                            appItemYpos = xml.readElementText().toInt();
                        }
                        if(xml.name() == "width")
                        {
                            appItemWidth = xml.readElementText().toInt();
                        }
                        if(xml.name() == "height")
                        {
                            appItemHeight = xml.readElementText().toInt();
                        }
                        if(xml.name() == "textEn")
                        {
                            appItemTextEn = xml.readElementText();
                        }
                        if(xml.name() == "textZh")
                        {
                            appItemTextZh = xml.readElementText();
                        }
                        if(xml.name() == "textTw")
                        {
                            appItemTextTw = xml.readElementText();
                        }
                        if(xml.name() == "normalpicpath")
                        {
                            appItemNormalPicPath = xml.readElementText();
                        }
                        if(xml.name() == "pushedpicpath")
                        {
                            appItemText = xml.readElementText();
                        }
                        if(xml.name() == "visible")
                        {
                            tempStr = xml.readElementText();
                            // default value is true
                            if (tempStr.compare("false") == 0) {
                                visible = false;
                            }
                        }
                        if(xml.name() == "enable")
                        {
                            tempStr = xml.readElementText();
                            // default value is true
                            if (tempStr.compare("false") == 0) {
                                enable = false;
                            }

                        }
                        if(xml.name() == "normalcolor")
                        {
                            normalTextColor = xml.readElementText();
                        }
                        if(xml.name() == "disablecolor")
                        {
                            disableTextColor = xml.readElementText();
                        }
                    }
                    xml.readNext();
                }

                //std::string apptext = appItemTextEn.toStdString();
                //LOGI(TAG_HOME, "QXmlStreamReader::EndElement, appItemTextEn(%s)\n", apptext.c_str());
                appItem = new CAppItem(appItemId, appItemPage, appItemIndex, appItemXpos,appItemYpos,appItemWidth, appItemHeight,
                                       appItemTextEn, appItemTextZh, appItemTextTw,
                                       appItemNormalPicPath, appItemText, visible, enable, normalTextColor, disableTextColor);
                //appItem->toString();
// #ifndef ATC_BT_SUPPORT
//                     if (appItemId == CAPPBaseObj::APPID_BT
//                             || appItemId == CAPPBaseObj::APPID_BTSPP) {
//                         LOGW(TAG_HOME, "NO SUPPORT BT\n");
//                         continue;
//                     }
// #endif
#ifndef ATC_GNSS_SUPPORT
                    if (appItemId == CAPPBaseObj::APPID_GPS) {
                        LOGW(TAG_HOME, "NO SUPPORT GNSS\n");
                        continue;
                    }
#endif


                #ifndef CARPLAY_SUPPORT
                    if (appItemId == CAPPBaseObj::APPID_CARPLAY_SETTINGS
                            || appItemId == CAPPBaseObj::APPID_CARPLAY_APP) {
                        continue;
                    }
                #endif

                #ifndef ANDROIDAUTO_SUPPORT
                    if (appItemId == CAPPBaseObj::APPID_ANDROIDAUTO_SETTINGS
                            || appItemId == CAPPBaseObj::APPID_ANDROIDAUTO_APP) {
                        continue;
                    }
                #endif

                // #ifndef AVM_SUPPORT
                //     if (appItemId == CAPPBaseObj::APPID_AVM) {
                //         continue;
                //     }
                // #endif

                m_appList.append(appItem);
            }
        }
    }
    LOGI(TAG_HOME, "QXmlStreamReader::atEnd\n");

    if(xml.hasError())
    {
         LOGI(TAG_HOME, "Parse XML file error\r\n");
    }

    xml.clear();

    return true;
}


//--------------------------------------------------------------------------------
//  @Function Name : CAppItemParser::member
//
//  @Param :
//        - int index
//
//  @Return : CAppItem*
//
//  @Description :get AppItem at index
//
//---------------------------------------------------------------------------------
CAppItem* CAppItemParser::member(int index)const
{

    if (index < 0 || index >= membersCount()) {
        return NULL;
    }

    return m_appList.at(index);
}


//--------------------------------------------------------------------------------
//  @Function Name : CAppItemParser::membersCount
//
//  @Param : - None
//
//  @Return : int
//
//  @Description :get AppItem count
//
//---------------------------------------------------------------------------------
int CAppItemParser::membersCount() const
{
    return m_appList.size();
}

//--------------------------------------------------------------------------------
//  @Function Name : CAppItemParser::isEmpty
//
//  @Param : - None
//
//  @Return : BOOL
//
//  @Description :judge the member list. If the member list is empty, return true;
//                       vice versa,if the member list is not empty, then return false.
//
//---------------------------------------------------------------------------------
bool CAppItemParser::isEmpty()
{
    return (m_appList.size() == 0)? true: false;
}
