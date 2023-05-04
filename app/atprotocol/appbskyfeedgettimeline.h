#ifndef APPBSKYFEEDGETTIMELINE_H
#define APPBSKYFEEDGETTIMELINE_H

#include "accessatprotocol.h"
#include "../atprotocol/lexicons.h"

class AppBskyFeedGetTimeline : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetTimeline(QObject *parent = nullptr);

    void getTimeline();

    const QList<AppBskyFeedDefs::FeedViewPost> *feedList() const;

private:
    virtual void parseJson(const QString reply_json);

    QList<AppBskyFeedDefs::FeedViewPost> m_feedList;

    void copyAuthor(const QJsonObject &json_author, AppBskyActorDefs::ProfileViewBasic &author);
};

#endif // APPBSKYFEEDGETTIMELINE_H
