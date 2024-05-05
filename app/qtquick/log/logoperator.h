#ifndef LOGOPERATOR_H
#define LOGOPERATOR_H

#include <QObject>

class LogOperator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged FINAL)
    Q_PROPERTY(QString did READ did WRITE setDid NOTIFY didChanged FINAL)

public:
    explicit LogOperator(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void clear();

    bool running() const;
    void setRunning(bool newRunning);
    QString service() const;
    void setService(const QString &newService);
    QString did() const;
    void setDid(const QString &newDid);

signals:
    void errorOccured(const QString &code, const QString &message);
    void finished(bool success);
    void runningChanged();
    void serviceChanged();
    void didChanged();

private:
    bool m_running;
    QString m_service;
    QString m_did;
};

#endif // LOGOPERATOR_H
