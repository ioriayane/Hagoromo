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

AbstractPostSelector::AbstractPostSelector(QObject *parent) : QObject { parent }
{
    if (parent != nullptr && parent->metaObject()->superClass() != nullptr) {
        if (strcmp(parent->metaObject()->superClass()->className(), "AbstractPostSelector") == 0) {
            setDid(reinterpret_cast<AbstractPostSelector *>(parent)->did());
        }
    }
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
    m_children.append(child);
}

void AbstractPostSelector::setFollowing(const QStringList &following)
{
    for (auto child : children()) {
        child->setFollowing(following);
    }
}

void AbstractPostSelector::setFollowers(const QStringList &followers)
{
    for (auto child : children()) {
        child->setFollowers(followers);
    }
}

const QList<AbstractPostSelector *> &AbstractPostSelector::children() const
{
    return m_children;
}

QString AbstractPostSelector::getRepo(const QJsonObject &object) const
{
    return object.value("repo").toString();
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
