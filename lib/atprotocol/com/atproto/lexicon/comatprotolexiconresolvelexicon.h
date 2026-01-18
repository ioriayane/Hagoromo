#ifndef COMATPROTOLEXICONRESOLVELEXICON_H
#define COMATPROTOLEXICONRESOLVELEXICON_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoLexiconResolveLexicon : public AccessAtProtocol
{
public:
    explicit ComAtprotoLexiconResolveLexicon(QObject *parent = nullptr);

    void resolveLexicon(const QString &nsid);

    const QString &cid() const;
    const AtProtocolType::ComAtprotoLexiconSchema::Main &schema() const;
    const QString &uri() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_cid;
    AtProtocolType::ComAtprotoLexiconSchema::Main m_schema;
    QString m_uri;
};

}

#endif // COMATPROTOLEXICONRESOLVELEXICON_H
