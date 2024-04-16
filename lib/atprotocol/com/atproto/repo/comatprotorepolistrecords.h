#ifndef COMATPROTOREPOLISTRECORDS_H
#define COMATPROTOREPOLISTRECORDS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoRepoListRecords : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoListRecords(QObject *parent = nullptr);

    void listRecords(const QString &repo, const QString &collection, const int limit,
                     const QString &cursor, const bool reverse);

    const QList<AtProtocolType::ComAtprotoRepoListRecords::Record> &recordList() const;

protected:
    virtual bool parseJson(bool success, const QString reply_json);

private:
    QList<AtProtocolType::ComAtprotoRepoListRecords::Record> m_recordList;
};

}

#endif // COMATPROTOREPOLISTRECORDS_H
