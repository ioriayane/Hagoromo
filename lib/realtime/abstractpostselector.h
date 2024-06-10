#ifndef ABSTRACTPOSTSELECTOR_H
#define ABSTRACTPOSTSELECTOR_H

#include <QObject>
#include <QJsonObject>

namespace RealtimeFeed {

struct UserInfo
{
    QString did;
    QString rkey;
};

class AbstractPostSelector : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPostSelector(QObject *parent = nullptr);
    ~AbstractPostSelector();

    virtual bool judge(const QJsonObject &object) = 0;
    virtual QString toString();

    static AbstractPostSelector *create(const QJsonObject &selector, QObject *parent);
    void appendChildSelector(AbstractPostSelector *child);
    virtual bool needFollowing() const;
    virtual bool needFollowers() const;

    virtual void setFollowing(const QList<UserInfo> &following);
    virtual void setFollowers(const QList<UserInfo> &followers);

    static QStringList getOperationUris(const QJsonObject &object);

    QString did() const;
    void setDid(const QString &newDid);
    QString name() const;
    void setName(const QString &newName);
    bool isArray() const;
    void setIsArray(bool newIsArray);
    bool parentIsArray() const;
    void setParentIsArray(bool newParentIsArray);
    bool ready() const;
    void setReady(bool newReady);

signals:
    void selected(const QJsonObject &object);

protected:
    const QList<AbstractPostSelector *> &children() const;

    bool isTarget(const QJsonObject &object) const;
    bool isMy(const QJsonObject &object) const;
    QString getRepo(const QJsonObject &object) const;
    QJsonObject getOperation(const QJsonObject &object, const QString &id) const;
    QJsonObject getBlock(const QJsonObject &object, const QString &path) const;

    QString extractRkey(const QString &path) const;

private:
    QList<AbstractPostSelector *> m_children;
    QString m_did;
    QString m_name;
    bool m_isArray;
    bool m_parentIsArray;
    bool m_ready;
};
}

#endif // ABSTRACTPOSTSELECTOR_H
