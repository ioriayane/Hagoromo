#ifndef COMATPROTOREPOLISTRECORDS_H
#define COMATPROTOREPOLISTRECORDS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class ComAtprotoRepoListRecords : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoListRecords(QObject *parent = nullptr);

    bool listRecords(const QString &repo, const QString &collection, const int limit,
                     const QString &cursor, const QString &rkeyStart, const QString &rkeyEnd);
    bool listLikes(const QString &repo, const QString &cursor);
    bool listReposts(const QString &repo, const QString &cursor);
    bool listListItems(const QString &repo, const QString &cursor);

    const QList<AtProtocolType::ComAtprotoRepoListRecords::Record> *recordList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ComAtprotoRepoListRecords::Record> m_recordList;
};

}

#endif // COMATPROTOREPOLISTRECORDS_H
