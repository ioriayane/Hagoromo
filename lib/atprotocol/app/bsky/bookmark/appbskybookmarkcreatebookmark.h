#ifndef APPBSKYBOOKMARKCREATEBOOKMARK_H
#define APPBSKYBOOKMARKCREATEBOOKMARK_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyBookmarkCreateBookmark : public AccessAtProtocol
{
public:
    explicit AppBskyBookmarkCreateBookmark(QObject *parent = nullptr);

    void createBookmark(const QString &uri, const QString &cid);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYBOOKMARKCREATEBOOKMARK_H
