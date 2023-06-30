#ifndef SEARCHPOSTS_H
#define SEARCHPOSTS_H

#include "atprotocol/accessatprotocol.h"
#include "search.h"

namespace SearchInterface {

class SearchPosts : public AtProtocolInterface::AccessAtProtocol
{
    Q_OBJECT
public:
    explicit SearchPosts(QObject *parent = nullptr);

    void search(const QString &text);

    const QList<SearchType::ViewPost> *viewPostList() const;

private:
    virtual void parseJson(bool success, const QString reply_json);

    QList<SearchType::ViewPost> m_viewPostList;
};

}

#endif // SEARCHPOSTS_H
