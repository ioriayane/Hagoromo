#ifndef PLCDIRECTORY_H
#define PLCDIRECTORY_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class PlcDirectory : public AccessAtProtocol
{
public:
    explicit PlcDirectory(QObject *parent = nullptr);

    void directory(const QString &did);

    const AtProtocolType::ComAtprotoServerDefs::DidDoc &didDoc() const;
    QString serviceEndpoint() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ComAtprotoServerDefs::DidDoc m_didDoc;
};

}

#endif // PLCDIRECTORY_H
