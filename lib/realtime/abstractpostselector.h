#ifndef ABSTRACTPOSTSELECTOR_H
#define ABSTRACTPOSTSELECTOR_H

#include <QObject>
#include <QJsonObject>

namespace RealtimeFeed {

enum class OperationActionType : int { Create, Delete };

struct UserInfo
{
    QString did;
    QString rkey;
    QString handle;
    QString display_name;
};

struct OperationInfo
{
    OperationActionType action = OperationActionType::Create;
    QString cid;
    QString uri;
    bool is_like = false;
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
    virtual bool has(const QString &type) const;
    virtual bool needFollowing() const;
    virtual bool needFollowers() const;
    virtual bool needListMembers() const;

    virtual void setFollowing(const QList<UserInfo> &following);
    virtual void setFollowers(const QList<UserInfo> &followers);
    virtual void setListMembers(const QString &list_uri, const QList<UserInfo> &members);
    virtual QStringList getListUris() const;
    virtual UserInfo getUser(const QString &did) const;

    static QStringList getOperationUris(const QJsonObject &object);
    QList<OperationInfo> getOperationInfos(const QJsonObject &object);
    bool isReaction(const QJsonObject &object);
    void appendReactionCandidate(const QString &uri);

    int getNodeCount() const;
    AbstractPostSelector *itemAt(int &index);
    AbstractPostSelector *remove(AbstractPostSelector *s);
    int index(AbstractPostSelector *s, int index = 0) const;
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

    int imageCount() const;
    void setImageCount(int newImageCount);
    bool hasMovie() const;
    void setHasMovie(bool newHasMovie);
    bool hasQuote() const;
    void setHasQuote(bool newHasQuote);
    bool hasImage() const;
    void setHasImage(bool newHasImage);
    QString listUri() const;
    void setListUri(const QString &newListUri);
    QString listName() const;
    void setListName(const QString &newListName);
    int movieCount() const;
    void setMovieCount(int newMovieCount);
    int quoteCondition() const;
    void setQuoteCondition(int newQuoteCondition);
    bool isRepost() const;
    void setIsRepost(bool newIsRepost);
    int repostCondition() const;
    void setRepostCondition(int newRepostCondition);

signals:
    void selected(const QJsonObject &object);
    void reacted(const QJsonObject &object);

protected:
    const QList<AbstractPostSelector *> &children() const;

    bool isTarget(const QJsonObject &object) const;
    bool isMy(const QJsonObject &object) const;
    bool matchImageCondition(const QJsonObject &object) const;
    bool matchMovieCondition(const QJsonObject &object) const;
    bool matchQuoteCondition(const QJsonObject &object) const;
    bool matchRepostCondition(const QJsonObject &object) const;
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

    bool m_hasImage;
    int m_imageCount; // -1: >=1, 0: nothing only, 1~4: match count
    bool m_hasMovie;
    int m_movieCount; // 0: nothing only, 1: match count
    bool m_hasQuote;
    int m_quoteCondition; // 0: only, 1: exclude
    bool m_isRepost;
    int m_repostCondition; // 0: only, 1: exclude
    QString m_listUri;
    QString m_listName;
    QStringList m_reationCandidates;
};
}

#endif // ABSTRACTPOSTSELECTOR_H
