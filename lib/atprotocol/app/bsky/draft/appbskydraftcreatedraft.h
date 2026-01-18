#ifndef APPBSKYDRAFTCREATEDRAFT_H
#define APPBSKYDRAFTCREATEDRAFT_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyDraftCreateDraft : public AccessAtProtocol
{
public:
    explicit AppBskyDraftCreateDraft(QObject *parent = nullptr);

    void createDraft(const QJsonObject &draft);

    const QString &id() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_id;
};

}

#endif // APPBSKYDRAFTCREATEDRAFT_H
