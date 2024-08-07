#include "abstractpostselector.h"
#include "andpostselector.h"
#include "orpostselector.h"
#include "xorpostselector.h"
#include "notpostselector.h"
#include "followingpostselector.h"
#include "followerspostselector.h"
#include "mepostselector.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace RealtimeFeed {

AbstractPostSelector::AbstractPostSelector(QObject *parent)
    : QObject { parent }, m_isArray(false), m_parentIsArray(true), m_ready(false)
{
}

AbstractPostSelector::~AbstractPostSelector()
{
    qDebug().noquote() << this << "~AbstractPostSelector()" << name() << did();
}

QString AbstractPostSelector::toString()
{

    QString ret;
    ret += parentIsArray() ? "{" : "";
    ret += QString("\"%1\":").arg(name());
    ret += isArray() ? "[" : "{";
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
        key = "follower";
        current = new FollowersPostSelector(parent);
    } else if (selector.contains("me")) {
        key = "me";
        current = new MePostSelector(parent);
    } else if (selector.contains("lists")) {
        key = "lists";
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
    }
    return current;
}

void AbstractPostSelector::appendChildSelector(AbstractPostSelector *child)
{
    if (child == nullptr)
        return;
    if (m_children.contains(child))
        return;
    child->setParentIsArray(isArray());
    m_children.append(child);
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

QString AbstractPostSelector::name() const
{
    return m_name;
}

void AbstractPostSelector::setName(const QString &newName)
{
    m_name = newName;
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
