#include "comatprotorepouploadblob.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QCoreApplication>
#include <QDir>

namespace AtProtocolInterface {

ComAtprotoRepoUploadBlob::ComAtprotoRepoUploadBlob(QObject *parent)
    : AccessAtProtocol { parent } { }

void ComAtprotoRepoUploadBlob::uploadBlob(const QString &path)
{
    postWithImage(QStringLiteral("xrpc/com.atproto.repo.uploadBlob"), compress(path));
}

void ComAtprotoRepoUploadBlob::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
    } else {
        qDebug() << reply_json;
        QString cid = json_doc.object()
                              .value("blob")
                              .toObject()
                              .value("ref")
                              .toObject()
                              .value("$link")
                              .toString();
        QString mimetype = json_doc.object().value("blob").toObject().value("mimeType").toString();
        int size = json_doc.object().value("blob").toObject().value("size").toInt();
        if (!cid.isEmpty() && !mimetype.isEmpty()) {
            m_cid = cid;
            m_mimeType = mimetype;
            m_size = size;
            success = true;
        }
    }

    emit finished(success);
}

QString ComAtprotoRepoUploadBlob::compress(const QString &path)
{
    QFileInfo info(path);
    if (info.size() < 1000000)
        return path;

    QString folder =
            QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation),
                                 QCoreApplication::applicationName());
    QDir dir(folder);
    dir.mkpath(folder);

    QImage src(path);
    QString new_path = QString("%1/%2.jpg").arg(folder, QUuid::createUuid().toString(QUuid::Id128));

    QFileInfo new_info(path);
    for (int quality = 90; quality >= 70; quality -= 5) {
        // qDebug() << new_info.size() << "/" << info.size() << "," << quality;
        if (new_info.size() < 1000000) {
            break;
        }
        src.save(new_path, nullptr, quality);
        new_info.setFile(new_path);
    }
    for (qreal ratio = 0.9; ratio >= 0.1; ratio -= 0.1) {
        // qDebug() << new_info.size() << "/" << info.size() << "," << src.width() << "x"
        //         << src.height();
        if (new_info.size() < 1000000) {
            break;
        }
        src.scaled(src.width() * ratio, src.height() * ratio, Qt::KeepAspectRatio)
                .save(new_path, nullptr, 70);
        new_info.setFile(new_path);
    }
    return new_path;
}

int ComAtprotoRepoUploadBlob::size() const
{
    return m_size;
}

QString ComAtprotoRepoUploadBlob::mimeType() const
{
    return m_mimeType;
}

QString ComAtprotoRepoUploadBlob::cid() const
{
    return m_cid;
}

}
