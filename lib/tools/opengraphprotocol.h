#ifndef OPENGRAPHPROTOCOL_H
#define OPENGRAPHPROTOCOL_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QRegularExpression>
#include <QDomElement>

class OpenGraphProtocol : public QObject
{
    Q_OBJECT
public:
    explicit OpenGraphProtocol(QObject *parent = nullptr);

    void getData(const QString &url);
    void downloadThumb(const QString &path);
    static QString decodeHtml(const QString &encoded);

    QString uri() const;
    void setUri(const QString &newUri);
    QString title() const;
    void setTitle(const QString &newTitle);
    QString description() const;
    void setDescription(const QString &newDescription);
    QString thumb() const;
    void setThumb(const QString &newThumb);

signals:
    void finished(bool success);
    void finishedDownload(bool success);

private:
    bool parse(const QByteArray &data, const QString &src_uri);
    QString extractCharset(const QString &data) const;
    void rebuildHtml(const QString &text, QDomDocument &doc) const;
    bool rebuildTag(QString text, QDomElement &element) const;

    QRegularExpression m_rxMeta;
    QHash<QString, QString> m_listOfRedirectAllowed; // QHash<元URL, 先URL>

    QString m_uri;
    QString m_title;
    QString m_description;
    QString m_thumb;
};

#endif // OPENGRAPHPROTOCOL_H
