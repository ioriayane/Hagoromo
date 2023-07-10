#ifndef OPENGRAPHPROTOCOL_H
#define OPENGRAPHPROTOCOL_H

#include <QNetworkAccessManager>
#include <QObject>

class OpenGraphProtocol : public QObject
{
    Q_OBJECT
public:
    explicit OpenGraphProtocol(QObject *parent = nullptr);

    void getData(const QString &url);

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

private:
    QNetworkAccessManager m_manager;

    bool parse(const QByteArray &data);
    QString extractCharset(const QByteArray &data) const;

    QString m_uri;
    QString m_title;
    QString m_description;
    QString m_thumb;
};

#endif // OPENGRAPHPROTOCOL_H
