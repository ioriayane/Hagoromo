#ifndef APPBSKYDRAFTGETDRAFTS_H
#define APPBSKYDRAFTGETDRAFTS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyDraftGetDrafts : public AccessAtProtocol
{
public:
    explicit AppBskyDraftGetDrafts(QObject *parent = nullptr);

    void getDrafts(const int limit, const QString &cursor);

    const QList<AtProtocolType::AppBskyDraftDefs::DraftView> &draftsList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyDraftDefs::DraftView> m_draftsList;
};

}

#endif // APPBSKYDRAFTGETDRAFTS_H
