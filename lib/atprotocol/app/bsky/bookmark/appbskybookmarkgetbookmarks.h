#ifndef APPBSKYBOOKMARKGETBOOKMARKS_H
#define APPBSKYBOOKMARKGETBOOKMARKS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyBookmarkGetBookmarks : public AccessAtProtocol
{
public:
    explicit AppBskyBookmarkGetBookmarks(QObject *parent = nullptr);

    void getBookmarks(const qint64 limit, const QString &cursor);

    const QList<AtProtocolType::AppBskyBookmarkDefs::BookmarkView> &bookmarksList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyBookmarkDefs::BookmarkView> m_bookmarksList;
};

}

#endif // APPBSKYBOOKMARKGETBOOKMARKS_H
