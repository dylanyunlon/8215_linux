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

#ifndef CAPPITEM_H
#define CAPPITEM_H
#include <QObject>
#include <QString>

#define  DEFAULT_NORMAL_TEXT_COLOR     "#FFFFFFFF"
#define  DEFAULT_DISABLE_TEXT_COLOR    "#FF808080"


class CAppItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int appid READ getId WRITE setId)
    Q_PROPERTY(int page READ page WRITE setPage)
    Q_PROPERTY(int index READ index WRITE setIndex)
    Q_PROPERTY(int xpos READ xpos WRITE setXpos)
    Q_PROPERTY(int ypos READ ypos WRITE setYpos)
    Q_PROPERTY(int width READ width WRITE setWidth)
    Q_PROPERTY(int height READ height WRITE setHeight)
    Q_PROPERTY(QString textEn READ getTextEn WRITE setTextEn)
    Q_PROPERTY(QString textZh READ getTextZh WRITE setTextZh)
    Q_PROPERTY(QString textTw READ getTextTw WRITE setTextTw)
    Q_PROPERTY(QString normalPicpath READ getNormalPicPath WRITE setNormalPicPath)
    Q_PROPERTY(QString text READ getText WRITE setText)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
    Q_PROPERTY(bool enable READ isEnable WRITE setEnable)
    Q_PROPERTY(QString normalTextColor READ getNormalTextColor WRITE setNormalTextColor)
    Q_PROPERTY(QString disableTextColor READ getDisableTextColor WRITE setDisableTextColor)
    Q_PROPERTY(READ toString)

public:
    CAppItem(int id,int page,int index,
             int xpos, int ypos, int width, int height,
             QString text_en, QString text_zh, QString text_tw,
             QString normalPicPath, QString text,bool visible,
             bool enable, QString normalTextColor, QString disableTextColor);
    explicit CAppItem(QObject *parent = 0);
    ~CAppItem();
    void initAppItem(QObject *rootObject);

public :
    Q_INVOKABLE bool toString();

    Q_INVOKABLE void setId(const int appid);
    Q_INVOKABLE void setPage(const int page);
    Q_INVOKABLE void setIndex(const int index);
    Q_INVOKABLE void setXpos(const int xpos);
    Q_INVOKABLE void setYpos(const int ypos);
    Q_INVOKABLE void setWidth(const int width);
    Q_INVOKABLE void setHeight(const int height);
    Q_INVOKABLE void setTextEn(const QString textEn);
    Q_INVOKABLE void setTextZh(const QString textZh);
    Q_INVOKABLE void setTextTw(const QString textTw);
    Q_INVOKABLE void setNormalPicPath(const QString normalPicpath);
    Q_INVOKABLE void setText(const QString text);
    Q_INVOKABLE void setVisible(bool visible);
    Q_INVOKABLE void setEnable(bool enable);
    Q_INVOKABLE void setNormalTextColor(const QString textColor);
    Q_INVOKABLE void setDisableTextColor(const QString textColor);

    Q_INVOKABLE int getId(void) const;
    Q_INVOKABLE int page(void) const;
    Q_INVOKABLE int index(void) const;
    Q_INVOKABLE int xpos(void) const;
    Q_INVOKABLE int ypos(void) const;
    Q_INVOKABLE int width(void) const;
    Q_INVOKABLE int height(void) const;
    Q_INVOKABLE QString getTextEn(void) const;
    Q_INVOKABLE QString getTextZh(void) const;
    Q_INVOKABLE QString getTextTw(void) const;
    Q_INVOKABLE QString getNormalPicPath(void) const;
    Q_INVOKABLE QString getText(void) const;
    Q_INVOKABLE bool isVisible(void) const;
    Q_INVOKABLE bool isEnable(void) const;
    Q_INVOKABLE QString getNormalTextColor(void) const;
    Q_INVOKABLE QString getDisableTextColor(void) const;
    

public slots:

    void onIdChange(int id);
    void onPageChange(int page);
    void onIndexChange(int index);
    void onxPosChange(int xpos);
    void onyPosChange(int ypos);
    void onWidthChange(int width);
    void onHeightChange(int height);
    void onTextEnChange(QString textEn);
    void onTextZhChange(QString textZh);
    void onTextTwChange(QString textTw);
    void onNormalPicChange(QString normalPicpath);
    void onTextChange(QString text);

signals:
    void sendIdChange(int);
    void sendPageChange(int);
    void sendIndexChange(int);
    void sendXposChange(int);
    void sendYposChange(int);
    void sendWidthChange(int);
    void sendHeightChange(int);
    void sendTextEnChange(QString);
    void sendTextZhChange(QString);
    void sendTextTwChange(QString);
    void sendNormalPicPathChange(QString);
    void sendTextChange(QString);

private:
    int m_id;
    int m_page;
    int m_index;
    int m_xpos;
    int m_ypos;
    int m_width;
    int m_height;
    QString m_textEn;
    QString m_textZh;
    QString m_textTw;
    QString m_normalPicPath;
    QString m_text;
    QObject *m_rootObject;
    bool m_visible;
    bool m_enable;
    QString m_normalTextColor;
    QString m_disableTextColor;
};

#endif // CAPPITEM_H
