#ifndef ATPABSTRACTLISTMODEL_H
#define ATPABSTRACTLISTMODEL_H

#include "../atprotocol/accessatprotocol.h"

#include <QAbstractListModel>
#include <QObject>

class AtpAbstractListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
public:
    explicit AtpAbstractListModel(QObject *parent = nullptr);

    virtual Q_INVOKABLE void getLatest() = 0;

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    bool running() const;
    void setRunning(bool newRunning);

signals:
    void runningChanged();

protected:
    QString formatDateTime(const QString &value) const;

private:
    AtProtocolInterface::AccountData m_account;

    bool m_running;
};

#endif // ATPABSTRACTLISTMODEL_H
