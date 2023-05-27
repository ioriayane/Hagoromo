#ifndef ATPABSTRACTLISTMODEL_H
#define ATPABSTRACTLISTMODEL_H

#include "atprotocol/accessatprotocol.h"

#include <QAbstractListModel>
#include <QObject>
#include <QTimer>

class AtpAbstractListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool autoLoading READ autoLoading WRITE setAutoLoading NOTIFY autoLoadingChanged)
    Q_PROPERTY(int loadingInterval READ loadingInterval WRITE setLoadingInterval NOTIFY
                       loadingIntervalChanged)

public:
    explicit AtpAbstractListModel(QObject *parent = nullptr);

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    virtual Q_INVOKABLE int indexOf(const QString &cid) const = 0;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid) = 0;
    Q_INVOKABLE void translate(const QString &cid);

    bool running() const;
    void setRunning(bool newRunning);
    bool autoLoading() const;
    void setAutoLoading(bool newAutoLoading);
    int loadingInterval() const;
    void setLoadingInterval(int newLoadingInterval);

signals:
    void runningChanged();
    void autoLoadingChanged();
    void loadingIntervalChanged();

public slots:
    virtual Q_INVOKABLE void getLatest() = 0;

protected:
    QString formatDateTime(const QString &value) const;

    QHash<QString, QString> m_translations; // QHash<cid, translation>

private:
    QTimer m_timer;
    AtProtocolInterface::AccountData m_account;

    bool m_running;
    int m_loadingInterval;
};

#endif // ATPABSTRACTLISTMODEL_H
