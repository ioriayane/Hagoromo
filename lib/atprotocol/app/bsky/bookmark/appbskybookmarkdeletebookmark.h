#ifndef APPBSKYBOOKMARKDELETEBOOKMARK_H
#define APPBSKYBOOKMARKDELETEBOOKMARK_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyBookmarkDeleteBookmark : public AccessAtProtocol
{
public:
    explicit AppBskyBookmarkDeleteBookmark(QObject *parent = nullptr);

    void deleteBookmark(const QString &uri);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYBOOKMARKDELETEBOOKMARK_H
