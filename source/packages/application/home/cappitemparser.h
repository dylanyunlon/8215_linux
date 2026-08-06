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
 
#ifndef CAPPITEMPARSER_H
#define CAPPITEMPARSER_H

#include "cappitem.h"

class CAppItemParser : public QObject
{
    Q_OBJECT
    
public:
    explicit CAppItemParser(QObject *parent = 0);
    ~CAppItemParser();

public:
    bool parseXML(QString file_name);
    Q_INVOKABLE CAppItem*member(int index) const;
    Q_INVOKABLE int membersCount(void) const;
    bool isEmpty();
    void initAppItemList(QObject *rootObject);

private:
    QList<CAppItem*> m_appList;
    QObject *m_rootObject;
};

#endif // CAPPITEMPARSER_H
