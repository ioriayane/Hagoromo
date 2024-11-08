#include "editselectorlistmodel.h"

#include <QJsonDocument>

using namespace RealtimeFeed;

EditSelectorListModel::EditSelectorListModel(QObject *parent)
    : QAbstractListModel { parent }, m_selector(nullptr)
{
}

int EditSelectorListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_selector == nullptr) {
        return 0;
    } else {
        return m_selector->getNodeCount();
    }
}

QVariant EditSelectorListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<EditSelectorListModelRoles>(role));
}

QVariant EditSelectorListModel::item(int row, EditSelectorListModelRoles role) const
{
    if (m_selector == nullptr) {
        return QVariant();
    }
    int index = row;
    const AbstractPostSelector *s = m_selector->itemAt(index);
    if (s == nullptr) {
        return QVariant();
    }

    if (role == TypeRole) {
        return s->type();
    } else if (role == DisplayTypeRole) {
        return s->displayType();
    } else if (role == CanContainRole) {
        return s->canContain();
    } else if (role == IndentRole) {
        index = row;
        return m_selector->indentAt(index, 0);
    }
    return QVariant();
}

void EditSelectorListModel::appendChild(int row, const QString &type)
{
    if (m_selector == nullptr) {
        QJsonObject json;
        json.insert(type, QJsonObject());
        m_selector = AbstractPostSelector::create(json, this);
        if (m_selector != nullptr) {
            setValid(validate());
            emit dataChanged(index(0), index(0));
        }
    } else {
        int i = row;
        AbstractPostSelector *s = m_selector->itemAt(i);
        if (s == nullptr) {
            return;
        }
        if (!s->canContain().contains(type)) {
            return;
        }
        QJsonObject json;
        json.insert(type, QJsonObject());
        s->appendChildSelector(AbstractPostSelector::create(json, s));
        setValid(validate());
        emit dataChanged(index(row), index(rowCount() - 1));
    }
}

void EditSelectorListModel::remove(int row)
{
    if (m_selector == nullptr)
        return;

    if (row == 0) {
        clear();
    } else {
        int i = row;
        AbstractPostSelector *s = m_selector->itemAt(i);
        if (s == nullptr) {
            return;
        }
        int remove_count = s->getNodeCount();
        beginRemoveRows(QModelIndex(), row, row + remove_count - 1);
        m_selector->remove(s);
        endRemoveRows();
    }
}

void EditSelectorListModel::clear()
{
    if (m_selector == nullptr)
        return;

    int count = rowCount();
    if (count > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    }
    m_selector->deleteLater();
    m_selector = nullptr;
    if (count > 0) {
        endRemoveRows();
    }
    setValid(validate());
}

bool EditSelectorListModel::validate() const
{
    if (m_selector == nullptr) {
        return false;
    } else {
        return m_selector->validate();
    }
}

QString EditSelectorListModel::toJson() const
{
    if (m_selector == nullptr) {
        return QString();
    } else {
        return m_selector->toString();
    }
}

QHash<int, QByteArray> EditSelectorListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[TypeRole] = "type";
    roles[DisplayTypeRole] = "displayType";
    roles[CanContainRole] = "canContain";
    roles[IndentRole] = "indent";

    return roles;
}

void EditSelectorListModel::loadSelector()
{
    if (selectorJson().isEmpty()) {
        return;
    }

    QJsonObject json = QJsonDocument::fromJson(selectorJson().toUtf8()).object();
    if (json.isEmpty()) {
        return;
    }

    if (m_selector != nullptr) {
        m_selector->deleteLater();
        m_selector = nullptr;
    }
    m_selector = AbstractPostSelector::create(json, this);

    setValid(validate());

    emit dataChanged(index(0), index(rowCount() - 1));
}

QString EditSelectorListModel::selectorJson() const
{
    return m_selectorJson;
}

void EditSelectorListModel::setSelectorJson(const QString &newSelectorJson)
{
    if (m_selectorJson == newSelectorJson)
        return;
    m_selectorJson = newSelectorJson;
    emit selectorJsonChanged();

    loadSelector();
}

bool EditSelectorListModel::valid() const
{
    return m_valid;
}

void EditSelectorListModel::setValid(bool newValid)
{
    if (m_valid == newValid)
        return;
    m_valid = newValid;
    emit validChanged();
}
