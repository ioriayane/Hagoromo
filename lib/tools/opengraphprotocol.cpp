#include "opengraphprotocol.h"

#include <QNetworkReply>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QUrl>

// https://ogp.me/

OpenGraphProtocol::OpenGraphProtocol(QObject *parent) : QObject { parent }
{
    m_rxMeta = QRegularExpression(
            QString("(?:%1)|(?:%2)")
                    .arg("<meta (?:[ \\t]*?[a-zA-Z-_]+[ \\t]*?=[ \\t]*?\"?[^\"]*?\"?)+[ \\t]*?/?>",
                         "<title.+?</title[ \\t]*>"),
            QRegularExpression::CaseInsensitiveOption);

    m_listOfRedirectAllowed["youtu.be"] = "www.youtube.com";
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
            qCritical().noquote().nospace() << QString::fromUtf8(reply->readAll());
        } else {
            ret = parse(reply->readAll(), reply->url().toString());
        }
        emit finished(ret);
        reply->deleteLater();
        manager->deleteLater();
    });

    manager->setRedirectPolicy(QNetworkRequest::UserVerifiedRedirectPolicy);
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::redirected, [=](const QUrl &r_url) {
        QUrl b_url = url;
        qDebug() << "REDIRECT" << b_url.host() << r_url.host();
        if (b_url.host() == r_url.host()) {
            emit reply->redirectAllowed();
        } else if (m_listOfRedirectAllowed.value(b_url.host()) == r_url.host()) {
            emit reply->redirectAllowed();
        }
    });
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

    QDomDocument doc;
    rebuildHtml(ts.readAll(), doc);
    qDebug().noquote().nospace() << doc.toString();

    QDomElement root = doc.documentElement();
    QDomElement head = root.firstChildElement("head");

    setUri(src_uri);

    QDomElement element = head.firstChildElement();
    while (!element.isNull()) {
        if (element.tagName().toLower() == "meta") {
            QString property = element.attribute("property");
            QString content = element.attribute("content");
            if (property == "og:url") {
                if (content.startsWith("/")) {
                    QUrl uri(src_uri);
                    setUri(uri.toString(QUrl::RemovePath) + content);
                } else {
                    setUri(content);
                }
            } else if (property == "og:title") {
                setTitle(content);
            } else if (property == "og:description") {
                setDescription(content);
            } else if (property == "og:image") {
                // ダウンロードしてローカルパスに置換が必要
                if (content.startsWith("/")) {
                    QUrl uri(src_uri);
                    setThumb(uri.toString(QUrl::RemovePath) + content);
                } else {
                    setThumb(content);
                }
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

    return ret;
}

QString OpenGraphProtocol::extractCharset(const QString &data) const
{
    QString charset = "utf-8";

    QRegularExpressionMatch match = m_rxMeta.match(data);
    if (match.capturedTexts().isEmpty())
        return charset;

    QDomDocument doc;
    QString result;
    int pos;
    while ((pos = match.capturedStart()) != -1) {
        QDomElement element = doc.createElement("meta");
        if (rebuildTag(match.captured(), element)) {
            if (element.tagName().toLower() == "meta") {
                if (element.attribute("http-equiv").toLower().trimmed() == "content-type") {
                    QString content = element.attribute("content");
                    QStringList items = content.split(";");
                    bool exist = false;
                    for (const QString &item : qAsConst(items)) {
                        QStringList parts = item.trimmed().split("=");
                        if (parts.length() != 2)
                            continue;
                        if (parts[0].toLower().trimmed() == "charset") {
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

void OpenGraphProtocol::rebuildHtml(const QString &text, QDomDocument &doc) const
{
    QRegularExpressionMatch match = m_rxMeta.match(text);
    if (match.capturedTexts().isEmpty())
        return;

    QDomElement html = doc.createElement("html");
    QDomElement head = doc.createElement("head");

    int pos;
    while ((pos = match.capturedStart()) != -1) {
        QDomElement element = doc.createElement("meta");
        if (rebuildTag(match.captured(), element)) {
            head.appendChild(element);
        }
        match = m_rxMeta.match(text, pos + match.capturedLength());
    }

    html.appendChild(head);
    doc.appendChild(html);

    return;
}

bool OpenGraphProtocol::rebuildTag(QString text, QDomElement &element) const
{
    QChar c;
    int state = 0;
    bool in_quote = false;
    // 0:属性より前
    // 1:属性名
    // 2:スペース（=より後ろ）
    // 3:属性値
    QString result;
    QStringList names;
    QStringList values;

    text = text.trimmed();

    int close_tag_pos = -1;
    if (text.toLower().startsWith("<title")) {
        element.setTagName("title");
        close_tag_pos = text.indexOf(
                QRegularExpression("</title[ \\t]*>$", QRegularExpression::CaseInsensitiveOption));
    }
    if (!text.endsWith("/>") && !text.toLower().startsWith("<title")) {
        text.replace(">", "/>");
    }

    for (int i = 0; i < text.length(); i++) {
        c = text.at(i);
        if (state == 0) {
            if (c == ' ') {
                state = 1;
                names.append("");
                values.append("");
            }
            result += c;
        } else if (state == 1) {
            if (c == '/' || c == '>') {
                if (close_tag_pos > i) {
                    element.appendChild(element.toDocument().createTextNode(
                            text.mid(i + 1, close_tag_pos - (i + 1))));
                }
                break;
            } else if (c == '=') {
                state = 2;
                in_quote = false;
            } else {
                names.last().append(c);
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
                values.last().append(c);
            }
            result += c;
        } else if (state == 3) {
            if (in_quote) {
                if (c == '\"') {
                    state = 1;
                    in_quote = false;
                    names.append("");
                    values.append("");
                } else {
                    values.last().append(c);
                }
            } else {
                if (c == '/' || c == '>') {
                    if (close_tag_pos > i) {
                        element.appendChild(element.toDocument().createTextNode(
                                text.mid(i + 1, close_tag_pos - (i + 1))));
                    }
                    break;
                } else if (c == ' ') {
                    result += '\"';
                    state = 1;
                    names.append("");
                    values.append("");
                } else {
                    values.last().append(c);
                }
            }
            result += c;
        } else {
        }
    }

    if (names.length() == values.length() && !names.isEmpty()) {
        for (int i = 0; i < names.length(); i++) {
            if (!names.at(i).trimmed().isEmpty())
                element.setAttribute(names.at(i).trimmed(), values.at(i).trimmed());
        }
        return true;
    } else if (element.tagName() == "title") {
        return true;
    } else {
        return false;
    }
}
