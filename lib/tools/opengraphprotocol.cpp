#include "opengraphprotocol.h"

#include <QNetworkReply>
#include <QDomDocument>
#include <QDomElement>
#include <QPointer>
#include <QFile>

// https://ogp.me/

OpenGraphProtocol::OpenGraphProtocol(QObject *parent) : QObject { parent } { }

void OpenGraphProtocol::getData(const QString &url)
{
    QPointer<OpenGraphProtocol> aliving(this);

    QNetworkRequest request((QUrl(url)));

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << "OpenGraphProtocol reply" << reply->error() << reply->url();
        if (aliving) {
            bool ret = (reply->error() == QNetworkReply::NoError);
            if (!ret) {
                qCritical() << QString::fromUtf8(reply->readAll());
            } else {
                ret = parse(reply->readAll());
            }
            emit finished(ret);
        }
        reply->deleteLater();
        manager->deleteLater();
    });
    manager->get(request);
}

void OpenGraphProtocol::downloadThumb()
{
    if (thumb().isEmpty()) {
        emit finished(false);
        return;
    }

    QPointer<OpenGraphProtocol> aliving(this);

    qDebug() << thumb();
    QNetworkRequest request((QUrl(thumb())));

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << "downloadThumb reply" << reply->error() << reply->url();
        if (aliving) {
            bool ret = (reply->error() == QNetworkReply::NoError);
            if (ret) {
                if (m_thumbPath.open()) {
                    qDebug() << m_thumbPath.fileName();
                    m_thumbPath.write(reply->readAll());
                    m_thumbPath.close();
                    ret = true;
                }
            }
            emit finished(ret);
        }
        reply->deleteLater();
        manager->deleteLater();
    });
    manager->get(request);
}

QString OpenGraphProtocol::uri() const
{
    return m_uri;
}

void OpenGraphProtocol::setUri(const QString &newUri)
{
    m_uri = newUri;
}

QString OpenGraphProtocol::title() const
{
    return m_title;
}

void OpenGraphProtocol::setTitle(const QString &newTitle)
{
    m_title = newTitle;
}

QString OpenGraphProtocol::description() const
{
    return m_description;
}

void OpenGraphProtocol::setDescription(const QString &newDescription)
{
    m_description = newDescription;
}

QString OpenGraphProtocol::thumb() const
{
    return m_thumb;
}

void OpenGraphProtocol::setThumb(const QString &newThumb)
{
    m_thumb = newThumb;
}

QString OpenGraphProtocol::thumbLocal() const
{
    return m_thumbPath.fileName();
}

bool OpenGraphProtocol::parse(const QByteArray &data)
{
    bool ret = false;
    QString charset = extractCharset(data);
    qDebug() << "charset" << charset;

    QTextStream ts(data);
    ts.setCodec(charset.toLatin1());

    QString errorMsg;
    int errorLine;
    int errorColumn;

    QDomDocument doc;
    if (!doc.setContent(ts.readAll(), true, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "parse" << errorMsg << ", Line=" << errorLine << ", Column=" << errorColumn;
    } else {
        QDomElement root = doc.documentElement();
        QDomElement head = root.firstChildElement("head");

        QDomElement element = head.firstChildElement("meta");
        while (!element.isNull()) {
            QString property = element.attribute("property");
            QString content = element.attribute("content");
            if (property == "og:url") {
                setUri(content);
            } else if (property == "og:title") {
                setTitle(content);
            } else if (property == "og:description") {
                setDescription(content);
            } else if (property == "og:image") {
                // ダウンロードしてローカルパスに置換が必要
                setThumb(content);
            }
            element = element.nextSiblingElement("meta");
        }

        ret = true;
    }
    return ret;
}

QString OpenGraphProtocol::extractCharset(const QByteArray &data) const
{
    QString charset = "utf-8";

    QString errorMsg;
    int errorLine;
    int errorColumn;
    QDomDocument doc;

    if (!doc.setContent(data, true, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "extractCharset" << errorMsg << ", Line=" << errorLine
                 << ", Column=" << errorColumn;
    } else {
        QDomElement root = doc.documentElement();
        QDomElement head = root.firstChildElement("head");

        QDomElement element = head.firstChildElement("meta");
        while (!element.isNull()) {
            if (element.attribute("http-equiv") == "content-type") {
                QString content = element.attribute("content");
                QStringList items = content.split(";");
                bool exist = false;
                for (const QString &item : qAsConst(items)) {
                    QStringList parts = item.trimmed().split("=");
                    if (parts.length() != 2)
                        continue;
                    if (parts[0] == "charset") {
                        charset = parts[1].trimmed();
                        exist = true;
                        break;
                    }
                }
                if (exist)
                    break;
            } else {
                QString temp = element.attribute("charset");
                if (!temp.isEmpty()) {
                    charset = temp;
                    break;
                }
            }
            element = element.nextSiblingElement("meta");
        }
    }

    return charset;
}
