#ifndef OPENGRAPHPROTOCOL_H
#define OPENGRAPHPROTOCOL_H

#include <QObject>

class OpenGraphProtocol : public QObject
{
    Q_OBJECT
public:
    explicit OpenGraphProtocol(QObject *parent = nullptr);
    ~OpenGraphProtocol();

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
    class Private;
    Private *d;
    Q_DISABLE_COPY(OpenGraphProtocol)
};

#endif // OPENGRAPHPROTOCOL_H
