#ifndef ABSTRACTPOSTSELECTOR_H
#define ABSTRACTPOSTSELECTOR_H

#include <QObject>
#include <QJsonObject>

class AbstractPostSelector : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object) = 0;
    virtual QString toString() = 0;

    static AbstractPostSelector *create(const QJsonObject &selector, QObject *parent);
    void appendChildSelector(AbstractPostSelector *child);

    virtual void setFollowing(const QStringList &following);
    virtual void setFollowers(const QStringList &followers);
    QString did() const;
    void setDid(const QString &newDid);

signals:
    void selected(const QJsonObject &object);

protected:
    const QList<AbstractPostSelector *> &children() const;

    QString getRepo(const QJsonObject &object) const;

private:
    QList<AbstractPostSelector *> m_children;
    QString m_did;
};

#endif // ABSTRACTPOSTSELECTOR_H
