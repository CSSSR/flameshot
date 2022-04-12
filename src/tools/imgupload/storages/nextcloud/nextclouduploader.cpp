// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "nextclouduploader.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/history.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QBuffer>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QShortcut>
#include <QUrlQuery>

NextcloudUploader::NextcloudUploader(const QPixmap& capture, QWidget* parent)
  : ImgUploaderBase(capture, parent)
{
    m_NetworkAM = new QNetworkAccessManager(this);
    connect(m_NetworkAM,
            &QNetworkAccessManager::finished,
            this,
            &NextcloudUploader::handleReply);
}

void NextcloudUploader::handleReply(QNetworkReply* reply)
{
    spinner()->deleteLater();
    m_currentImageName.clear();
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        QJsonObject data = json[QStringLiteral("data")].toObject();
        //setImageURL(data[QStringLiteral("link")].toString());

        auto deleteToken = data[QStringLiteral("deletehash")].toString();

        // save history
        m_currentImageName = imageURL().toString();
        int lastSlash = m_currentImageName.lastIndexOf("/");
        if (lastSlash >= 0) {
            m_currentImageName = m_currentImageName.mid(lastSlash + 1);
        }

        // save image to history
        History history;
        m_currentImageName =
          history.packFileName("imgur", deleteToken, m_currentImageName);
        history.save(pixmap(), m_currentImageName);

        emit uploadOk(imageURL());
    } else {
        setInfoLabelText(reply->errorString());
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void NextcloudUploader::upload()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap().save(&buffer, "PNG");

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("title"), QStringLiteral(""));
    QString filename = FileNameHandler().parsedPattern();

	QString str("https://nextcloud.csssr.com/remote.php/dav/files/%1/%2/%3.%4");

	QUrl url(str
			.arg(ConfigHandler().nextcloudLogin())
			.arg(ConfigHandler().nextcloudUid())
			.arg(filename)
			.arg(ConfigHandler().saveAsFileExtension()));

    url.setQuery(urlQuery);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/application/x-www-form-urlencoded");

	QString username = ConfigHandler().nextcloudLogin();
	QString password = ConfigHandler().nextcloudPassword();
	QString concatenated = username + ":" + password;
	QByteArray data = concatenated.toLocal8Bit().toBase64();
	QString headerData = "Basic " + data;
	request.setRawHeader("Authorization", headerData.toLocal8Bit());

	QString publicUrl("https://nextcloud.csssr.com/%1/%2.%3");
	setImageURL(publicUrl
			.arg(ConfigHandler().nextcloudUid())
			.arg(filename)
			.arg(ConfigHandler().saveAsFileExtension()));

    m_NetworkAM->put(request, byteArray);
}

void NextcloudUploader::deleteImage(const QString& fileName,
                                const QString& deleteToken)
{
    Q_UNUSED(fileName)
    bool successful = QDesktopServices::openUrl(
      QUrl(QStringLiteral("https://imgur.com/delete/%1").arg(deleteToken)));
    if (!successful) {
        notification()->showMessage(tr("Unable to open the URL."));
    }

    emit deleteOk();
}
