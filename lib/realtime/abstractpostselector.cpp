#include "abstractpostselector.h"
#include <QDebug>

AbstractPostSelector::AbstractPostSelector(QObject *parent) : QObject { parent }
{
    if (parent != nullptr && parent->metaObject()->superClass() != nullptr) {
        if (strcmp(parent->metaObject()->superClass()->className(), "AbstractPostSelector") == 0) {
            setDid(reinterpret_cast<AbstractPostSelector *>(parent)->did());
        }
    }
}

void AbstractPostSelector::appendChildSelector(AbstractPostSelector *child)
{
    if (child == nullptr)
        return;
    if (m_children.contains(child))
        return;
    m_children.append(child);
}

const QList<AbstractPostSelector *> &AbstractPostSelector::children() const
{
    return m_children;
}

QString AbstractPostSelector::did() const
{
    return m_did;
}

void AbstractPostSelector::setDid(const QString &newDid)
{
    m_did = newDid;
}
