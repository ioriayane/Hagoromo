#ifndef APPBSKYLABELERGETSERVICES_H
#define APPBSKYLABELERGETSERVICES_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyLabelerGetServices : public AccessAtProtocol
{
public:
    explicit AppBskyLabelerGetServices(QObject *parent = nullptr);

    const QList<AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed> *labelerViewDetails();

    void getServices(const QList<QString> &dids, const bool detailed);

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed> m_labelerViewDetails;
};

}

#endif // APPBSKYLABELERGETSERVICES_H
