#ifndef APPBSKYEMBEDGETEMBEDEXTERNALVIEW_H
#define APPBSKYEMBEDGETEMBEDEXTERNALVIEW_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyEmbedGetEmbedExternalView : public AccessAtProtocol
{
public:
    explicit AppBskyEmbedGetEmbedExternalView(QObject *parent = nullptr);

    void getEmbedExternalView(const QString &url, const QList<QString> &uris);

    const AtProtocolType::AppBskyEmbedExternal::View &view() const;
    const QList<AtProtocolType::ComAtprotoRepoStrongRef::Main> &associatedRefsList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyEmbedExternal::View m_view;
    QList<AtProtocolType::ComAtprotoRepoStrongRef::Main> m_associatedRefsList;
};

}

#endif // APPBSKYEMBEDGETEMBEDEXTERNALVIEW_H
