#include "abstractpostselector.h"
#include "andpostselector.h"
#include "orpostselector.h"
#include "xorpostselector.h"
#include "notpostselector.h"
#include "followingpostselector.h"
#include "followerspostselector.h"
#include "mepostselector.h"
#include "listpostsselector.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace RealtimeFeed {

AbstractPostSelector::AbstractPostSelector(QObject *parent)
    : QObject { parent },
      m_isArray(false),
      m_parentIsArray(true),
      m_ready(false),
      m_hasImage(false),
      m_imageCount(0),
      m_hasMovie(false),
      m_hasQuote(false)
{
}

AbstractPostSelector::~AbstractPostSelector()
{
    qDebug().noquote() << this << "~AbstractPostSelector()" << type() << did();
}

QString AbstractPostSelector::toString()
{

    QString ret;
    ret += parentIsArray() ? "{" : "";
    ret += QString("\"%1\":").arg(type());
    ret += isArray() ? "[" : "{";
    if (!isArray()) {
        QString temp;
        if (!listUri().isEmpty()) {
            temp += QString("\"uri\":\"%1\"").arg(listUri());
            temp += QString(",\"name\":\"%1\"").arg(listName());
        }
        if (hasImage()) {
            if (!temp.isEmpty())
                temp += ",";
            temp += QString("\"image\":{\"has\":true,\"count\":%1}").arg(imageCount());
        }
        if (hasMovie()) {
            if (!temp.isEmpty())
                temp += ",";
            temp += QString("\"movie\":{\"has\":true}");
        }
        if (hasQuote()) {
            if (!temp.isEmpty())
                temp += ",";
            temp += QString("\"quote\":{\"has\":true}");
        }
        ret += temp;
    }
    int i = 0;
    for (auto child : children()) {
        if (i > 0) {
            ret += ",";
        }
        ret += child->toString();
        i++;
    }
    ret += isArray() ? "]" : "}";
    ret += parentIsArray() ? "}" : "";

    return ret;
}

bool AbstractPostSelector::validate() const
{
    for (auto child : children()) {
        if (!child->validate()) {
            return false;
        }
    }
    return true;
}

AbstractPostSelector *AbstractPostSelector::create(const QJsonObject &selector, QObject *parent)
{
    AbstractPostSelector *current = nullptr;
    QString key;

    if (selector.contains("and")) {
        key = "and";
        current = new AndPostSelector(parent);
    } else if (selector.contains("or")) {
        key = "or";
        current = new OrPostSelector(parent);
    } else if (selector.contains("xor")) {
        key = "xor";
        current = new XorPostSelector(parent);
    } else if (selector.contains("not")) {
        key = "not";
        current = new NotPostSelector(parent);
    } else if (selector.contains("following")) {
        key = "following";
        current = new FollowingPostSelector(parent);
    } else if (selector.contains("followers")) {
        key = "followers";
        current = new FollowersPostSelector(parent);
    } else if (selector.contains("me")) {
        key = "me";
        current = new MePostSelector(parent);
    } else if (selector.contains("list")) {
        key = "list";
        current = new ListPostsSelector(parent);
    }

    if (current == nullptr)
        return current;

    if (key == "and" || key == "or" || key == "xor") {
        for (const auto item : selector.value(key).toArray()) {
            AbstractPostSelector *child = AbstractPostSelector::create(item.toObject(), current);
            if (child != nullptr) {
                current->appendChildSelector(child);
            }
        }
    } else if (key == "not") {
        AbstractPostSelector *child =
                AbstractPostSelector::create(selector.value(key).toObject(), current);
        if (child != nullptr) {
            current->appendChildSelector(child);
        }
    } else {
        QJsonObject child_selector = selector.value(key).toObject();
        if (key == "list") {
            current->setListUri(child_selector.value("uri").toString());
            current->setListName(child_selector.value("name").toString());
        }
        current->setHasImage(child_selector.value("image").toObject().value("has").toBool(false));
        current->setImageCount(child_selector.value("image").toObject().value("count").toInt(0));
        current->setHasMovie(child_selector.value("movie").toObject().value("has").toBool(false));
        current->setHasQuote(child_selector.value("quote").toObject().value("has").toBool(false));
    }
    return current;
}

void AbstractPostSelector::appendChildSelector(AbstractPostSelector *child)
{
    if (child == nullptr)
        return;
    if (m_children.contains(child))
        return;
    if (!canContain().contains(child->type()))
        return;
    child->setParentIsArray(isArray());
    m_children.append(child);
}

QStringList AbstractPostSelector::canContain() const
{
    return QStringList() << "following"
                         << "followers"
                         << "list"
                         << "me"
                         << "and"
                         << "or"
                         << "not"
                         << "xor";
}

bool AbstractPostSelector::has(const QString &type) const
{
    for (auto child : children()) {
        if (child->type() == type) {
            return true;
        }
    }
    return false;
}

bool AbstractPostSelector::needFollowing() const
{
    for (auto child : children()) {
        if (child->needFollowing()) {
            return true;
        }
    }
    return false;
}

bool AbstractPostSelector::needFollowers() const
{
    for (auto child : children()) {
        if (child->needFollowers()) {
            return true;
        }
    }
    return false;
}

void AbstractPostSelector::setFollowing(const QList<UserInfo> &following)
{
    for (auto child : children()) {
        child->setFollowing(following);
    }
}

void AbstractPostSelector::setFollowers(const QList<UserInfo> &followers)
{
    for (auto child : children()) {
        child->setFollowers(followers);
    }
}

void AbstractPostSelector::setListMembers(const QList<UserInfo> &members)
{
    for (auto child : children()) {
        child->setListMembers(members);
    }
}

UserInfo AbstractPostSelector::getUser(const QString &did) const
{
    UserInfo info;

    if (did == this->did()) {
        info.did = did;
        info.handle = handle();
        info.display_name = displayName();
    } else {
        for (auto child : children()) {
            info = child->getUser(did);
            if (!info.did.isEmpty()) {
                return info;
            }
        }
    }

    return info;
}

QStringList AbstractPostSelector::getOperationUris(const QJsonObject &object)
{
    QStringList uris;
    QString repo = object.value("repo").toString();
    if (repo.isEmpty())
        return uris;

    for (const auto item : object.value("ops").toArray()) {
        if (item.toObject().value("action").toString() != "create") {
            continue;
        }
        QString path = item.toObject().value("path").toString();
        if (!path.isEmpty()) {
            uris.append(QString("at://%1/%2").arg(repo, path));
        }
    }
    return uris;
}

QList<OperationInfo> AbstractPostSelector::getOperationInfos(const QJsonObject &object)
{
    QList<OperationInfo> infos;
    QString repo = getRepo(object);
    if (repo.isEmpty())
        return infos;

    for (const auto item : object.value("ops").toArray()) {
        if (item.toObject().value("action").toString() != "create") {
            continue;
        }
        OperationInfo info;
        QString path = item.toObject().value("path").toString();
        QString cid = item.toObject().value("cid").toObject().value("$link").toString();
        if (!path.isEmpty() && !cid.isEmpty()) {
            if (path.startsWith("app.bsky.feed.repost/")) {
                QJsonObject block = getBlock(object, path);
                if (!block.isEmpty()) {
                    info.cid = block.value("value")
                                       .toObject()
                                       .value("subject")
                                       .toObject()
                                       .value("cid")
                                       .toString();
                    info.uri = block.value("value")
                                       .toObject()
                                       .value("subject")
                                       .toObject()
                                       .value("uri")
                                       .toString();
                    UserInfo user_info = getUser(repo);

                    if (!info.cid.isEmpty() && !info.uri.isEmpty()) {
                        info.is_repost = true;
                        info.reposted_by = repo;
                        info.reposted_by_handle = user_info.handle;
                        info.reposted_by_display_name = user_info.display_name;
                        infos.append(info);
                    }
                }
            } else {
                info.cid = cid;
                info.uri = QString("at://%1/%2").arg(repo, path);
                infos.append(info);
            }
        }
    }
    return infos;
}

int AbstractPostSelector::getNodeCount() const
{
    int count = 1;
    for (auto child : children()) {
        count += child->getNodeCount();
    }
    return count;
}

AbstractPostSelector *AbstractPostSelector::itemAt(int &index)
{
    if (index < 0) {
        return nullptr;
    } else if (index == 0) {
        return this;
    }
    index--;
    for (auto child : children()) {
        AbstractPostSelector *s = child->itemAt(index);
        if (s != nullptr) {
            return s;
        }
    }
    return nullptr;
}

// 消せたら自分を返す（つまり消したやつの親）
AbstractPostSelector *AbstractPostSelector::remove(AbstractPostSelector *s)
{
    AbstractPostSelector *ret = nullptr;
    if (m_children.contains(s)) {
        m_children.removeOne(s);
        s->deleteLater();
        ret = this;
    } else {
        for (auto child : children()) {
            ret = child->remove(s);
            if (ret != nullptr)
                break;
        }
    }
    return ret;
}

int AbstractPostSelector::index(AbstractPostSelector *s, int index) const
{
    if (s == this) {
        return index;
    }
    index++;
    for (auto child : children()) {
        int i = child->index(s, index);
        if (i >= 0) {
            return i;
        }
        index += child->getNodeCount();
    }
    return -1;
}

int AbstractPostSelector::indentAt(int &index, int current) const
{
    if (index < 0) {
        return -1;
    } else if (index == 0) {
        return current;
    }
    index--;
    for (auto child : children()) {
        int next = child->indentAt(index, current + 1);
        if (next > -1) {
            return next;
        }
    }
    return -1;
}

const QList<AbstractPostSelector *> &AbstractPostSelector::children() const
{
    return m_children;
}

bool AbstractPostSelector::isTarget(const QJsonObject &object) const
{
    QStringList targets;
    targets << "app.bsky.feed.post"
            << "app.bsky.feed.repost";
    for (const auto item : object.value("ops").toArray()) {
        if (item.toObject().value("action").toString() != "create") {
            continue;
        }
        QStringList path = item.toObject().value("path").toString().split("/");
        if (!path.isEmpty() && targets.contains(path.at(0))) {
            return true;
        }
    }
    return false;
}

bool AbstractPostSelector::isMy(const QJsonObject &object) const
{
    return (!did().isEmpty() && getRepo(object) == did());
}

QString AbstractPostSelector::getRepo(const QJsonObject &object) const
{
    return object.value("repo").toString();
}

QJsonObject AbstractPostSelector::getOperation(const QJsonObject &object, const QString &id) const
{
    for (const auto item : object.value("ops").toArray()) {
        QStringList path = item.toObject().value("path").toString().split("/");
        if (!path.isEmpty() && path.at(0) == id) {
            return item.toObject();
        }
    }
    return QJsonObject();
}

QJsonObject AbstractPostSelector::getBlock(const QJsonObject &object, const QString &path) const
{
    QString uri = QString("at://%1/%2").arg(getRepo(object), path);

    for (const auto item : object.value("blocks").toArray()) {
        if (item.toObject().value("uri").toString() == uri) {
            return item.toObject();
        }
    }
    return QJsonObject();
}

QString AbstractPostSelector::extractRkey(const QString &path) const
{
    QStringList items = path.split("/");
    if (items.length() > 1) {
        return items.last();
    } else {
        return QString();
    }
}

QString AbstractPostSelector::listName() const
{
    return m_listName;
}

void AbstractPostSelector::setListName(const QString &newListName)
{
    m_listName = newListName;
}

QString AbstractPostSelector::listUri() const
{
    return m_listUri;
}

void AbstractPostSelector::setListUri(const QString &newListUri)
{
    m_listUri = newListUri;
}

bool AbstractPostSelector::hasImage() const
{
    return m_hasImage;
}

void AbstractPostSelector::setHasImage(bool newHasImage)
{
    m_hasImage = newHasImage;
}

bool AbstractPostSelector::hasQuote() const
{
    return m_hasQuote;
}

void AbstractPostSelector::setHasQuote(bool newHasQuote)
{
    m_hasQuote = newHasQuote;
}

bool AbstractPostSelector::hasMovie() const
{
    return m_hasMovie;
}

void AbstractPostSelector::setHasMovie(bool newHasMovie)
{
    m_hasMovie = newHasMovie;
}

int AbstractPostSelector::imageCount() const
{
    return m_imageCount;
}

void AbstractPostSelector::setImageCount(int newImageCount)
{
    m_imageCount = newImageCount;
}

QString AbstractPostSelector::displayType() const
{
    return m_displayType;
}

void AbstractPostSelector::setDisplayType(const QString &newDisplayType)
{
    m_displayType = newDisplayType;
}

bool AbstractPostSelector::ready() const
{
    return m_ready;
}

void AbstractPostSelector::setReady(bool newReady)
{
    for (auto child : children()) {
        child->setReady(newReady);
    }
    m_ready = newReady;
}

bool AbstractPostSelector::parentIsArray() const
{
    return m_parentIsArray;
}

void AbstractPostSelector::setParentIsArray(bool newParentIsArray)
{
    m_parentIsArray = newParentIsArray;
}

bool AbstractPostSelector::isArray() const
{
    return m_isArray;
}

void AbstractPostSelector::setIsArray(bool newIsArray)
{
    m_isArray = newIsArray;
}

QString AbstractPostSelector::type() const
{
    return m_type;
}

void AbstractPostSelector::setType(const QString &newType)
{
    m_type = newType;
}

QString AbstractPostSelector::did() const
{
    return m_did;
}

void AbstractPostSelector::setDid(const QString &newDid)
{
    for (auto child : children()) {
        child->setDid(newDid);
    }
    m_did = newDid;
}

QString AbstractPostSelector::displayName() const
{
    return m_displayName;
}

void AbstractPostSelector::setDisplayName(const QString &newDisplayName)
{
    for (auto child : children()) {
        child->setDisplayName(newDisplayName);
    }
    m_displayName = newDisplayName;
}

QString AbstractPostSelector::handle() const
{
    return m_handle;
}

void AbstractPostSelector::setHandle(const QString &newHandle)
{
    for (auto child : children()) {
        child->setHandle(newHandle);
    }
    m_handle = newHandle;
}

}
