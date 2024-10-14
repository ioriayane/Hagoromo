#ifndef COMATPROTOREPOLISTRECORDSEX_H
#define COMATPROTOREPOLISTRECORDSEX_H

#include "atprotocol/com/atproto/repo/comatprotorepolistrecords.h"

namespace AtProtocolInterface {

class ComAtprotoRepoListRecordsEx : public ComAtprotoRepoListRecords
{
public:
    explicit ComAtprotoRepoListRecordsEx(QObject *parent = nullptr);

    void listLikes(const QString &repo, const QString &cursor);
    void listReposts(const QString &repo, const QString &cursor);
    void listListItems(const QString &repo, const QString &cursor);
    void listWhiteWindItems(const QString &repo, const QString &cursor);
    void listLinkatItems(const QString &repo, const QString &cursor);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOREPOLISTRECORDSEX_H
