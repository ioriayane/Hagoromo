#ifndef COMATPROTOMODERATIONCREATEREPORTEX_H
#define COMATPROTOMODERATIONCREATEREPORTEX_H

#include "atprotocol/com/atproto/moderation/comatprotomoderationcreatereport.h"

namespace AtProtocolInterface {

class ComAtprotoModerationCreateReportEx : public ComAtprotoModerationCreateReport
{
public:
    explicit ComAtprotoModerationCreateReportEx(QObject *parent = nullptr);

    void reportPost(const QString &uri, const QString &cid, const QString &reason);
    void reportAccount(const QString &did, const QString &reason);
    void reportMessage(const QString &did, const QString &convo_id, const QString &message_id,
                       const QString &text, const QString &reason);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOMODERATIONCREATEREPORTEX_H
