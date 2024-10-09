#ifndef COMATPROTOREPODESCRIBEREPO_H
#define COMATPROTOREPODESCRIBEREPO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoRepoDescribeRepo : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoDescribeRepo(QObject *parent = nullptr);

    void describeRepo(const QString &repo);

    const QString &handle() const;
    const QString &did() const;
    const QVariant &didDoc() const;
    const QStringList &collectionsList() const;
    const bool &handleIsCorrect() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_handle;
    QString m_did;
    QVariant m_didDoc;
    QStringList m_collectionsList;
    bool m_handleIsCorrect;
};

}

#endif // COMATPROTOREPODESCRIBEREPO_H
