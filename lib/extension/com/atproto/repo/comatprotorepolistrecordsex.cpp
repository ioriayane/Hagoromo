#include "comatprotorepolistrecordsex.h"

namespace AtProtocolInterface {

ComAtprotoRepoListRecordsEx::ComAtprotoRepoListRecordsEx(QObject *parent)
    : ComAtprotoRepoListRecords { parent }
{
}

void ComAtprotoRepoListRecordsEx::listLikes(const QString &repo, const QString &cursor)
{
    listRecords(repo, "app.bsky.feed.like", 50, cursor, false);
}

void ComAtprotoRepoListRecordsEx::listReposts(const QString &repo, const QString &cursor)
{
    listRecords(repo, "app.bsky.feed.repost", 50, cursor, false);
}

void ComAtprotoRepoListRecordsEx::listListItems(const QString &repo, const QString &cursor)
{
    listRecords(repo, "app.bsky.graph.listitem", 100, cursor, false);
}

void ComAtprotoRepoListRecordsEx::listWhiteWindItems(const QString &repo, const QString &cursor)
{
    listRecords(repo, "com.whtwnd.blog.entry", 10, cursor, false);
}

bool ComAtprotoRepoListRecordsEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoRepoListRecords::parseJson(success, reply_json);

    return success;
}

}
