#ifndef SEARCH_H
#define SEARCH_H

#include <QDateTime>
#include <QString>

namespace SearchType {

struct User
{
    QString did;
    QString handle;
};

struct Post
{
    QDateTime createdAt;
    QString text;
    QString user;
};

struct ViewPost
{
    QString tid;
    QString cid;
    User user;
    Post post;
};

}

#endif // SEARCH_H
