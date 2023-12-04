#ifndef COMATPROTOREPOPUTRECORD_H
#define COMATPROTOREPOPUTRECORD_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class ComAtprotoRepoPutRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoPutRecord(QObject *parent = nullptr);

    bool putRecord(const QString &repo, const QString &collection, const QString &rkey,
                   const bool validate, const QString &swapRecord, const QString &swapCommit,
                   const QJsonObject &record);

    bool profile(const AtProtocolType::Blob &avatar, const AtProtocolType::Blob &banner,
                 const QString &description, const QString &display_name, const QString &cid);
    bool list(const AtProtocolType::Blob &avatar, const QString &purpose,
              const QString &description, const QString &name, const QString &rkey);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOREPOPUTRECORD_H
