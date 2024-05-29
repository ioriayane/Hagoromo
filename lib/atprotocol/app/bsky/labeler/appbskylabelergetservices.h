#ifndef APPBSKYLABELERGETSERVICES_H
#define APPBSKYLABELERGETSERVICES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyLabelerGetServices : public AccessAtProtocol
{
public:
    explicit AppBskyLabelerGetServices(QObject *parent = nullptr);

    void getServices(const QList<QString> &dids, const bool detailed);

    const QList<AtProtocolType::AppBskyLabelerDefs::LabelerView> &viewsLabelerViewList() const;
    const QList<AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed> &
    viewsLabelerViewDetailedList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyLabelerDefs::LabelerView> m_viewsLabelerViewList;
    QList<AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed> m_viewsLabelerViewDetailedList;
};

}

#endif // APPBSKYLABELERGETSERVICES_H
