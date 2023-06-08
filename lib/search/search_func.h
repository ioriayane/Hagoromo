#ifndef SEARCH_FUNC_H
#define SEARCH_FUNC_H

#include "search.h"

#include <QJsonObject>

namespace SearchType {

void copyUser(const QJsonObject &src, User &dest);
void copyPost(const QJsonObject &src, Post &dest);
void copyViewPost(const QJsonObject &src, ViewPost &dest);

}

#endif // SEARCH_FUNC_H
