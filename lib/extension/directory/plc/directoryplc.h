#ifndef DIRECTORYPLC_H
#define DIRECTORYPLC_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class DirectoryPlc : public AccessAtProtocol
{
public:
    explicit DirectoryPlc(QObject *parent = nullptr);

    void directory(const QString &did);

    const AtProtocolType::ComAtprotoServerDefs::DidDoc &didDoc() const;
    QString serviceEndpoint() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ComAtprotoServerDefs::DidDoc m_didDoc;
};

}

#endif // DIRECTORYPLC_H
