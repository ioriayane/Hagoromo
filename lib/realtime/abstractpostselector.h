#ifndef ABSTRACTPOSTSELECTOR_H
#define ABSTRACTPOSTSELECTOR_H

#include <QObject>
#include <QJsonObject>

namespace RealtimeFeed {

struct UserInfo
{
    QString did;
    QString rkey;
    QString handle;
    QString display_name;
};

struct OperationInfo
{
    QString cid;
    QString uri;
    bool is_repost = false;
    QString reposted_by; // did
    QString reposted_by_handle;
    QString reposted_by_display_name;
};

class AbstractPostSelector : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPostSelector(QObject *parent = nullptr);
    ~AbstractPostSelector();

    virtual bool judge(const QJsonObject &object) = 0;
    virtual QString toString();
    virtual bool validate() const;

    static AbstractPostSelector *create(const QJsonObject &selector, QObject *parent);
    void appendChildSelector(AbstractPostSelector *child);
    virtual QStringList canContain() const;
    virtual bool needFollowing() const;
    virtual bool needFollowers() const;

    virtual void setFollowing(const QList<UserInfo> &following);
    virtual void setFollowers(const QList<UserInfo> &followers);
    virtual UserInfo getUser(const QString &did) const;

    static QStringList getOperationUris(const QJsonObject &object);
    QList<OperationInfo> getOperationInfos(const QJsonObject &object);

    int getNodeCount() const;
    AbstractPostSelector *itemAt(int &index);
    int indentAt(int &index, int current = 0) const;

    QString did() const; // account did
    void setDid(const QString &newDid);
    QString type() const;
    void setType(const QString &newType);
    bool isArray() const;
    void setIsArray(bool newIsArray);
    bool parentIsArray() const;
    void setParentIsArray(bool newParentIsArray);
    bool ready() const;
    void setReady(bool newReady);
    QString handle() const;
    void setHandle(const QString &newHandle);
    QString displayName() const;
    void setDisplayName(const QString &newDisplayName);

    QString displayType() const;
    void setDisplayType(const QString &newDisplayType);

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
    QString m_did; // of account
    QString m_handle; // of account
    QString m_displayName; // of account
    QString m_type;
    QString m_displayType;
    bool m_isArray;
    bool m_parentIsArray;
    bool m_ready;
};
}

#endif // ABSTRACTPOSTSELECTOR_H
