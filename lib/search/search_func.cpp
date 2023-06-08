#include "search_func.h"
#include "qjsonobject.h"

namespace SearchType {

void copyUser(const QJsonObject &src, User &dest)
{
    if (!src.isEmpty()) {
        dest.did = src.value("did").toString();
        dest.handle = src.value("handle").toString();
    }
}

void copyPost(const QJsonObject &src, Post &dest)
{
    if (!src.isEmpty()) {
        dest.text = src.value("text").toString();
        dest.user = src.value("user").toString();
        // dest.createdAt = QDateTime::fromSecsSinceEpoch(src.value("createdAt").toDouble());
    }
}

void copyViewPost(const QJsonObject &src, ViewPost &dest)
{
    if (!src.isEmpty()) {
        dest.cid = src.value("cid").toString();
        dest.tid = src.value("tid").toString();
        copyUser(src.value("user").toObject(), dest.user);
        copyPost(src.value("post").toObject(), dest.post);
    }
}

}
