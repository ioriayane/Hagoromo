#ifndef COMATPROTOREPOLISTRECORDS_H
#define COMATPROTOREPOLISTRECORDS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class ComAtprotoRepoListRecords : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoListRecords(QObject *parent = nullptr);

    void listRecords(const QString &repo, const QString &collection, const int limit,
                     const QString &cursor, const QString &rkeyStart, const QString &rkeyEnd);
    void listLikes(const QString &repo);
    void listReposts(const QString &repo);

    const QList<AtProtocolType::ComAtprotoRepoListRecords::Record> *recordList() const;

private:
    virtual void parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ComAtprotoRepoListRecords::Record> m_recordList;
};

}

#endif // COMATPROTOREPOLISTRECORDS_H
