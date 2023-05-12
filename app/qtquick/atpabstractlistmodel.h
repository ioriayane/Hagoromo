#ifndef ATPABSTRACTLISTMODEL_H
#define ATPABSTRACTLISTMODEL_H

#include "../atprotocol/accessatprotocol.h"

#include <QAbstractListModel>
#include <QObject>
#include <QTimer>

class AtpAbstractListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool autoLoading READ autoLoading WRITE setAutoLoading NOTIFY autoLoadingChanged)

public:
    explicit AtpAbstractListModel(QObject *parent = nullptr);

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    bool running() const;
    void setRunning(bool newRunning);
    bool autoLoading() const;
    void setAutoLoading(bool newAutoLoading);

signals:
    void runningChanged();
    void autoLoadingChanged();

public slots:
    virtual Q_INVOKABLE void getLatest() = 0;

protected:
    QString formatDateTime(const QString &value) const;

private:
    QTimer m_timer;
    AtProtocolInterface::AccountData m_account;

    bool m_running;
};

#endif // ATPABSTRACTLISTMODEL_H
