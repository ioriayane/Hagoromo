#ifndef APPBSKYGRAPHGETMUTES_H
#define APPBSKYGRAPHGETMUTES_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyGraphGetMutes : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetMutes(QObject *parent = nullptr);

    void getMutes(const int limit, const QString &cursor);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYGRAPHGETMUTES_H