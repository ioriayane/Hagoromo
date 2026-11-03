#ifndef COMATPROTOREPOGETRECORDEX_H
#define COMATPROTOREPOGETRECORDEX_H

#include "atprotocol/com/atproto/repo/comatprotorepogetrecord.h"

#include <QJsonObject>

namespace AtProtocolInterface {

class ComAtprotoRepoGetRecordEx : public ComAtprotoRepoGetRecord
{
public:
    explicit ComAtprotoRepoGetRecordEx(QObject *parent = nullptr);

    void profile(const QString &did);
    void list(const QString &did, const QString &rkey);
    void postGate(const QString &did, const QString &rkey);
    void skyBlurPost(const QString &did, const QString &rkey);
    void notificationDeclaration(const QString &did);
    void profileLink(const QString &did, const QString &rkey);

    const QJsonObject &rawValue() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QJsonObject m_rawValue;
};

}

#endif // COMATPROTOREPOGETRECORDEX_H
