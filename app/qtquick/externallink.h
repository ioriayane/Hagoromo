#ifndef EXTERNALLINK_H
#define EXTERNALLINK_H

#include <QObject>
#include <QTemporaryFile>

class ExternalLink : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri NOTIFY uriChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QString thumb READ thumb NOTIFY thumbChanged)
    Q_PROPERTY(QString thumbLocal READ thumbLocal NOTIFY thumbLocalChanged)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool valid READ valid WRITE setValid NOTIFY validChanged)

public:
    explicit ExternalLink(QObject *parent = nullptr);

    Q_INVOKABLE void getExternalLink(const QString &uri);

    QString uri() const;
    void setUri(const QString &newUri);
    QString title() const;
    void setTitle(const QString &newTitle);
    QString description() const;
    void setDescription(const QString &newDescription);
    QString thumb() const;
    void setThumb(const QString &newThumb);
    QString thumbLocal() const;
    void setThumbLocal(const QString &newThumbLocal);
    bool running() const;
    void setRunning(bool newRunning);
    bool valid() const;
    void setValid(bool newValid);

signals:
    void uriChanged();
    void titleChanged();
    void descriptionChanged();
    void thumbChanged();
    void thumbLocalChanged();
    void runningChanged();
    void validChanged();

private:
    QTemporaryFile m_thumbLocal;

    QString m_uri;
    QString m_title;
    QString m_description;
    QString m_thumb;
    bool m_running;
    bool m_valid;
};

#endif // EXTERNALLINK_H
