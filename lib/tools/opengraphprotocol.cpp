#include "opengraphprotocol.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QUrl>
#include <QTextCodec>
#include <QRegularExpression>

// https://ogp.me/

class OpenGraphProtocol::Private
{
public:
    Private(OpenGraphProtocol *parent);
    ~Private();

    QString uri() const;
    void setUri(const QString &newUri);
    QString title() const;
    void setTitle(const QString &newTitle);
    QString description() const;
    void setDescription(const QString &newDescription);
    QString thumb() const;
    void setThumb(const QString &newThumb);

    bool parse(const QByteArray &data, const QString &src_uri);
    QString extractCharset(const QString &data) const;
    void rebuildHtml(const QString &text, QDomDocument &doc) const;
    bool rebuildTag(QString text, QDomElement &element) const;

private:
    OpenGraphProtocol *q;

    QRegularExpression m_rxMeta;
    QHash<QString, QString> m_listOfRedirectAllowed; // QHash<元URL, 先URL>

    QString m_uri;
    QString m_title;
    QString m_description;
    QString m_thumb;
};

OpenGraphProtocol::Private::Private(OpenGraphProtocol *parent) : q(parent)
{
    m_rxMeta = QRegularExpression(
            QString("(?:%1)|(?:%2)")
                    .arg("<meta (?:[ \\t]*?[a-zA-Z-_]+[ \\t]*?=[ \\t]*?\"?[^\"]*?\"?)+[ \\t]*?/?>",
                         "<title.+?</title[ \\t]*>"),
            QRegularExpression::CaseInsensitiveOption);

    m_listOfRedirectAllowed["youtu.be"] = "www.youtube.com";
    m_listOfRedirectAllowed["microsoft.com"] = "microsoftonline.com";
    m_listOfRedirectAllowed["microsoftonline.com"] = "microsoft.com";
    m_listOfRedirectAllowed["amzn.asia"] = "www.amazon.co.jp";
    m_listOfRedirectAllowed["a.co"] = "www.amazon.com";
}

OpenGraphProtocol::Private::~Private() { }

OpenGraphProtocol::OpenGraphProtocol(QObject *parent) : QObject { parent }, d(new Private(this))
{
    qDebug().noquote() << this << "OpenGraphProtocol()";
    connect(this, &QObject::destroyed, [this]() { delete d; });
}

OpenGraphProtocol::~OpenGraphProtocol()
{
    qDebug().noquote() << this << "~OpenGraphProtocol()";
}

void OpenGraphProtocol::getData(const QString &url)
{
    d->setUri(QString());
    d->setTitle(QString());
    d->setDescription(QString());
    d->setThumb(QString());

    QNetworkRequest request((QUrl(url)));
    request.setRawHeader(QByteArray("accept"), QByteArray("*/*"));
    request.setRawHeader(QByteArray("sec-ch-ua-platform"), QByteArray("Windows"));
    request.setRawHeader(QByteArray("sec-fetch-dest"), QByteArray("document"));
    request.setRawHeader(QByteArray("sec-fetch-mode"), QByteArray("navigate"));
    request.setRawHeader(QByteArray("sec-fetch-site"), QByteArray("none"));
    request.setRawHeader(QByteArray("sec-fetch-user"), QByteArray("?1"));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
                      "like Gecko) Chrome/129.0.0.0 Safari/537.36");
    request.setTransferTimeout(60 * 1000);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << "OpenGraphProtocol reply" << reply->error() << reply->url();
        bool ret = (reply->error() == QNetworkReply::NoError);
        if (!ret) {
            qCritical().noquote().nospace() << QString::fromUtf8(reply->readAll());
        } else {
            ret = d->parse(reply->readAll(), reply->url().toString());
        }
        emit finished(ret);
        reply->deleteLater();
        manager->deleteLater();
    });

    // manager->setRedirectPolicy(QNetworkRequest::UserVerifiedRedirectPolicy);
    QNetworkReply *reply = manager->get(request);
    // connect(reply, &QNetworkReply::redirected, [=](const QUrl &r_url) {
    //     QUrl b_url = url;
    //     qDebug().noquote().nospace() << "REDIRECT " << b_url.scheme() << "://" << b_url.host()
    //                                  << " -> " << r_url.scheme() << "://" << r_url.host();
    //     if (!((b_url.scheme() == r_url.scheme())
    //           || (b_url.scheme() == "http" && r_url.scheme() == "https"))) {
    //         emit reply->finished();
    //     } else if (b_url.host() == r_url.host()) {
    //         emit reply->redirectAllowed();
    //     } else if (m_listOfRedirectAllowed.value(b_url.host()) == r_url.host()) {
    //         emit reply->redirectAllowed();
    //     } else {
    //         QStringList b_host_parts = b_url.host().split(".");
    //         QStringList r_host_parts = r_url.host().split(".");
    //         if (b_host_parts.length() < 2 || r_host_parts.length() < 2) {
    //             emit reply->finished();
    //         } else {
    //             QString b_host =
    //                     b_host_parts.at(b_host_parts.length() - 2) + "." + b_host_parts.last();
    //             QString r_host =
    //                     r_host_parts.at(r_host_parts.length() - 2) + "." + r_host_parts.last();
    //             if (b_host == r_host) {
    //                 emit reply->redirectAllowed();
    //             } else if (m_listOfRedirectAllowed.value(b_host) == r_host) {
    //                 emit reply->redirectAllowed();
    //             } else {
    //                 emit reply->finished();
    //             }
    //         }
    //     }
    // });
    connect(reply, &QNetworkReply::errorOccurred, [=](QNetworkReply::NetworkError code) {
        qDebug() << "Reply error:" << code << reply->request().url();
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

QString OpenGraphProtocol::decodeHtml(const QString &encoded)
{
    QRegularExpression re("&(#?\\w+);");
    QHash<QString, QString> htmlEntities = { { "&amp;", "&" },  { "&lt;", "<" },
                                             { "&gt;", ">" },   { "&quot;", "\"" },
                                             { "&apos;", "'" }, { "&nbsp;", " " } };
    QRegularExpressionMatch match = re.match(encoded);
    if (!match.hasMatch()) {
        return encoded;
    }
    QString decoded;
    int pos = 0;
    int start_pos = 0;
    while ((pos = match.capturedStart()) != -1) {
        if (!htmlEntities.contains(match.captured())) {
            decoded += encoded.mid(start_pos, match.capturedEnd() - start_pos + 1);
        } else {
            decoded += encoded.mid(start_pos, match.capturedStart() - start_pos);
            decoded += htmlEntities.value(match.captured());
        }

        start_pos = pos + match.capturedLength();
        match = re.match(encoded, start_pos);
    }
    decoded += encoded.mid(start_pos, encoded.length() - start_pos);

    return decoded;
}

QString OpenGraphProtocol::uri() const
{
    return d->uri();
}

void OpenGraphProtocol::setUri(const QString &newUri)
{
    d->setUri(newUri);
}

QString OpenGraphProtocol::title() const
{
    return d->title();
}

void OpenGraphProtocol::setTitle(const QString &newTitle)
{
    d->setTitle(newTitle);
}

QString OpenGraphProtocol::description() const
{
    return d->description();
}

void OpenGraphProtocol::setDescription(const QString &newDescription)
{
    d->setDescription(newDescription);
}

QString OpenGraphProtocol::thumb() const
{
    return d->thumb();
}

void OpenGraphProtocol::setThumb(const QString &newThumb)
{
    d->setThumb(newThumb);
}

QString OpenGraphProtocol::Private::uri() const
{
    return m_uri;
}

void OpenGraphProtocol::Private::setUri(const QString &newUri)
{
    m_uri = newUri;
}

QString OpenGraphProtocol::Private::title() const
{
    return m_title;
}

void OpenGraphProtocol::Private::setTitle(const QString &newTitle)
{
    m_title = newTitle;
}

QString OpenGraphProtocol::Private::description() const
{
    return m_description;
}

void OpenGraphProtocol::Private::setDescription(const QString &newDescription)
{
    m_description = newDescription;
}

QString OpenGraphProtocol::Private::thumb() const
{
    return m_thumb;
}

void OpenGraphProtocol::Private::setThumb(const QString &newThumb)
{
    m_thumb = newThumb;
}

bool OpenGraphProtocol::Private::parse(const QByteArray &data, const QString &src_uri)
{
    bool ret = false;
#if 0
    QString charset = extractCharset(QString::fromUtf8(data));
    qDebug() << "charset" << charset;

    QTextStream ts(data);
#    if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    ts.setCodec(charset.toLatin1());
#    else
#        error Qt5CompatのQTextCodecを使うように変更しないとECU-JPなどに対応できないはず
#    endif

    QDomDocument doc;
    rebuildHtml(ts.readAll(), doc);
#else
    QString charset = extractCharset(QString::fromUtf8(data));
    QTextCodec *codec = QTextCodec::codecForHtml(data, QTextCodec::codecForName(charset.toUtf8()));
    QDomDocument doc;
    rebuildHtml(codec->toUnicode(data), doc);
#endif
    qDebug().noquote().nospace() << doc.toString();

    QDomElement root = doc.documentElement();
    QDomElement head = root.firstChildElement("head");

    {
        QUrl uri(src_uri);
        setUri(uri.toString(QUrl::FullyEncoded));
    }

    QString temp_title;
    QString temp_description;
    QDomElement element = head.firstChildElement();
    while (!element.isNull()) {
        if (element.tagName().toLower() == "meta") {
            QString property = OpenGraphProtocol::decodeHtml(element.attribute("property"));
            if (property.isEmpty()) {
                property = OpenGraphProtocol::decodeHtml(element.attribute("name"));
            }
            QString content = OpenGraphProtocol::decodeHtml(element.attribute("content"));
            if (property == "og:url") {
                if (content.startsWith("/")) {
                    QUrl uri(src_uri);
                    setUri(uri.toString(QUrl::FullyEncoded | QUrl::RemovePath) + content);
                } else if (content.isEmpty()) {
                    // あえてタグを用意しているのに空はそのまま
                } else {
                    QUrl uri(content);
                    setUri(uri.toString(QUrl::FullyEncoded));
                }
            } else if (property == "og:title") {
                setTitle(content);
            } else if (property == "title") {
                temp_title = content;
            } else if (property == "og:description") {
                setDescription(content);
            } else if (property == "description") {
                temp_description = content;
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
            if (temp_title.isEmpty()) {
                temp_title = element.text();
            }
        }
        element = element.nextSiblingElement();
    }
    if (title().isEmpty() && !temp_title.isEmpty()) {
        setTitle(temp_title);
    }
    if (description().isEmpty() && !temp_description.isEmpty()) {
        setDescription(temp_description);
    }
    if (!uri().isEmpty() && !title().isEmpty()) {
        ret = true;
    }

    return ret;
}

QString OpenGraphProtocol::Private::extractCharset(const QString &data) const
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
                    for (const QString &item : std::as_const(items)) {
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

void OpenGraphProtocol::Private::rebuildHtml(const QString &text, QDomDocument &doc) const
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

bool OpenGraphProtocol::Private::rebuildTag(QString text, QDomElement &element) const
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
            } else if (c == '>') {
                if (close_tag_pos > i) {
                    element.appendChild(element.toDocument().createTextNode(
                            text.mid(i + 1, close_tag_pos - (i + 1))));
                }
                break;
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
        qDebug().noquote() << "rebuild:" << names << values << result
                           << element.toDocument().toString();
        return true;
    } else {
        return false;
    }
}
