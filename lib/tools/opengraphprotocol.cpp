#include "opengraphprotocol.h"

#include <QNetworkReply>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>

// https://ogp.me/

OpenGraphProtocol::OpenGraphProtocol(QObject *parent) : QObject { parent }
{
    m_rxMeta = QRegularExpression(
            QString("(?:%1)|(?:%2)")
                    .arg("<meta (?:[ \\t]*?[a-zA-Z-_]+[ \\t]*?=[ \\t]*?\"?[^\"]*?\"?)+[ \\t]*?/?>",
                         "<title.+?</title[ \\t]*>"),
            QRegularExpression::CaseInsensitiveOption);
}

void OpenGraphProtocol::getData(const QString &url)
{
    m_uri.clear();
    m_title.clear();
    m_description.clear();
    m_thumb.clear();

    QNetworkRequest request((QUrl(url)));

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << "OpenGraphProtocol reply" << reply->error() << reply->url();
        bool ret = (reply->error() == QNetworkReply::NoError);
        if (!ret) {
            qCritical() << QString::fromUtf8(reply->readAll());
        } else {
            ret = parse(reply->readAll(), reply->url().toString());
        }
        emit finished(ret);
        reply->deleteLater();
        manager->deleteLater();
    });
    manager->setRedirectPolicy(QNetworkRequest::SameOriginRedirectPolicy);
    manager->get(request);
}

void OpenGraphProtocol::downloadThumb(const QString &path)
{
    if (thumb().isEmpty()) {
        emit finishedDownload(false);
        return;
    }

    qDebug() << thumb();
    QNetworkRequest request((QUrl(thumb())));

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << "downloadThumb reply" << reply->error() << reply->url();
        bool ret = (reply->error() == QNetworkReply::NoError);
        if (ret) {
            QFile file(path);
            if (file.open(QFile::WriteOnly)) {
                qDebug() << "save download file:" << path;
                file.write(reply->readAll());
                file.close();
                ret = true;
            }
        }
        emit finishedDownload(ret);
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

bool OpenGraphProtocol::parse(const QByteArray &data, const QString &src_uri)
{
    bool ret = false;
    QString charset = extractCharset(QString::fromUtf8(data));
    qDebug() << "charset" << charset;

    QTextStream ts(data);
    ts.setCodec(charset.toLatin1());

    QString rebuild_text = rebuildHtml(ts.readAll());

    QString errorMsg;
    int errorLine;
    int errorColumn;

    QDomDocument doc;
    if (!doc.setContent(rebuild_text, false, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "parse" << errorMsg << ", Line=" << errorLine << ", Column=" << errorColumn;
        qDebug().noquote().nospace() << "--- rebuild_text -----------------";
        qDebug().noquote().nospace() << rebuild_text;
        qDebug().noquote().nospace() << "----------------------------------";
    } else {
        QDomElement root = doc.documentElement();
        QDomElement head = root.firstChildElement("head");

        setUri(src_uri);

        QDomElement element = head.firstChildElement();
        while (!element.isNull()) {
            if (element.tagName().toLower() == "meta") {
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
            } else if (element.tagName().toLower() == "title") {
                if (title().isEmpty()) {
                    setTitle(element.text());
                }
            }
            element = element.nextSiblingElement();
        }
        if (!uri().isEmpty() && !title().isEmpty()) {
            ret = true;
        }
    }
    return ret;
}

QString OpenGraphProtocol::extractCharset(const QString &data) const
{
    QString charset = "utf-8";

    QRegularExpressionMatch match = m_rxMeta.match(data);
    if (match.capturedTexts().isEmpty())
        return charset;

    QString errorMsg;
    int errorLine;
    int errorColumn;

    QDomDocument doc;
    QString result;
    int pos;
    while ((pos = match.capturedStart()) != -1) {
        result = rebuildTag(match.captured());

        if (!doc.setContent(result, false, &errorMsg, &errorLine, &errorColumn)) {
            qDebug() << "parse" << errorMsg << ", Line=" << errorLine << ", Column=" << errorColumn;
        } else {
            QDomElement element = doc.documentElement();
            if (element.tagName().toLower() == "meta") {
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
            }
        }

        match = m_rxMeta.match(data, pos + match.capturedLength());
    }

    return charset;
}

QString OpenGraphProtocol::rebuildHtml(const QString &text) const
{
    QRegularExpressionMatch match = m_rxMeta.match(text);
    if (match.capturedTexts().isEmpty())
        return text;

    QString errorMsg;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    QString result;
    int pos;
    while ((pos = match.capturedStart()) != -1) {
        QString temp = rebuildTag(match.captured());
        if (doc.setContent(temp, false, &errorMsg, &errorLine, &errorColumn)) {
            result += temp + "\n";
        }
        match = m_rxMeta.match(text, pos + match.capturedLength());
    }

    return QString("<html><head>%1</head></html>").arg(result);
}

QString OpenGraphProtocol::rebuildTag(QString text) const
{
    QChar c;
    int state = 0;
    bool in_quote = false;
    // 0:属性より前
    // 1:属性名
    // 2:スペース（=より後ろ）
    // 3:属性値
    QString result;

    if (!text.endsWith("/>") && !text.toLower().startsWith("<title")) {
        text.replace(">", "/>");
    }

    for (int i = 0; i < text.length(); i++) {
        c = text.at(i);
        if (state == 0) {
            if (c == ' ') {
                state = 1;
            }
            result += c;
        } else if (state == 1) {
            if (c == '=') {
                state = 2;
                in_quote = false;
            }
            result += c;
        } else if (state == 2) {
            if (c == '\"') {
                in_quote = true;
                state = 3;
            } else if (c != ' ') {
                result += '\"';
                in_quote = false;
                state = 3;
            }
            result += c;
        } else if (state == 3) {
            if (in_quote) {
                if (c == '\"') {
                    state = 1;
                    in_quote = false;
                }
            } else {
                if (c == ' ') {
                    result += '\"';
                    state = 1;
                }
            }
            result += c;
        } else {
        }
    }
    return result;
}
