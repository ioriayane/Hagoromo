#ifndef CHATBSKYGROUPGETJOINLINKPREVIEWS_H
#define CHATBSKYGROUPGETJOINLINKPREVIEWS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupGetJoinLinkPreviews : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupGetJoinLinkPreviews(QObject *parent = nullptr);

    void getJoinLinkPreviews(const QList<QString> &codes);

    const QList<AtProtocolType::ChatBskyGroupDefs::JoinLinkPreviewView> &
    joinLinkPreviewsJoinLinkPreviewViewList() const;
    const QList<AtProtocolType::ChatBskyGroupDefs::DisabledJoinLinkPreviewView> &
    joinLinkPreviewsDisabledJoinLinkPreviewViewList() const;
    const QList<AtProtocolType::ChatBskyGroupDefs::InvalidJoinLinkPreviewView> &
    joinLinkPreviewsInvalidJoinLinkPreviewViewList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyGroupDefs::JoinLinkPreviewView>
            m_joinLinkPreviewsJoinLinkPreviewViewList;
    QList<AtProtocolType::ChatBskyGroupDefs::DisabledJoinLinkPreviewView>
            m_joinLinkPreviewsDisabledJoinLinkPreviewViewList;
    QList<AtProtocolType::ChatBskyGroupDefs::InvalidJoinLinkPreviewView>
            m_joinLinkPreviewsInvalidJoinLinkPreviewViewList;
};

}

#endif // CHATBSKYGROUPGETJOINLINKPREVIEWS_H
