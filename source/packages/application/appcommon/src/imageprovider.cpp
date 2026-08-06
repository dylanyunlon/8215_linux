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
 

#include "appimageprovider.h"

#include <QDebug>

class AppImageProvider : public QQuickImageProvider {
	public:
		AppImageProvider(void);
 		virtual ~AppImageProvider(void);

		QPixmap requestPixmap(const QString &id, QSize *size, const QSize& requestedSize, bool requestedAutoTransform);
		QPixmap requestPixmap(const QString &id, QSize *size, const QSize& requestedSize);
};


AppImageProvider::AppImageProvider(void) : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
	//qDebug("[appimageprovider] AppImageProvider::AppImageProvider 1111111111111111, this = %p\n", this);
}


AppImageProvider::~AppImageProvider(void)
{
	//qDebug("[appimageprovider] AppImageProvider::~AppImageProvider 1111111111111111, this = %p\n", this);
}


QPixmap AppImageProvider::requestPixmap(const QString &id, QSize *size, const QSize& requestedSize)
{
	QPixmap pixmap;
	//qDebug("[appimageprovider] AppImageProvider::requestPixmap 1111111111111111\n");
        //qDebug() << "id = " << id;
        QString filename = QString(":/Res/") + id;
	//qDebug() << "file name = " << filename;

 	if (pixmap.load(filename)) {
		//qDebug("[appimageprovider] load file to pixmap 1111111111111\n");
		if (size) {
			size->setWidth(pixmap.width());
			size->setHeight(pixmap.height());
		}
  	} else {
		//qDebug("[appimageprovider] load file to pixmap 2222222222222\n");
	}

	return (pixmap);
}


QPixmap AppImageProvider::requestPixmap(const QString &id, QSize *size, const QSize& requestedSize, bool requestedAutoTransform)
{
	QPixmap *pixmap = new QPixmap();
	//qDebug("[appimageprovider] AppImageProvider::requestPixmap2 1111111111111111\n");

        return (*pixmap);
}


QQuickImageProvider *app_create_image_provider(void)
{
    return (new AppImageProvider());
}


void app_release_image_provoider(QQuickImageProvider *image_provider)
{
	if (image_provider) {
		delete image_provider;
	}
}

