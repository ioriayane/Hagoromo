#ifndef OPENGRAPHPROTOCOL_H
#define OPENGRAPHPROTOCOL_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QTemporaryFile>

class OpenGraphProtocol : public QObject
{
    Q_OBJECT
public:
    explicit OpenGraphProtocol(QObject *parent = nullptr);

    void getData(const QString &url);
    void downloadThumb();

    QString uri() const;
    void setUri(const QString &newUri);
    QString title() const;
    void setTitle(const QString &newTitle);
    QString description() const;
    void setDescription(const QString &newDescription);
    QString thumb() const;
    void setThumb(const QString &newThumb);
    QString thumbLocal() const;

signals:
    void finished(bool success);

private:
    bool parse(const QByteArray &data);
    QString extractCharset(const QByteArray &data) const;

    QTemporaryFile m_thumbPath;

    QString m_uri;
    QString m_title;
    QString m_description;
    QString m_thumb;
};

#endif // OPENGRAPHPROTOCOL_H
