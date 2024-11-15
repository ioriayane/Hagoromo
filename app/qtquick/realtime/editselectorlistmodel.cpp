#include "editselectorlistmodel.h"
#include "tools/accountmanager.h"

#include <QJsonDocument>

using namespace RealtimeFeed;

EditSelectorListModel::EditSelectorListModel(QObject *parent)
    : QAbstractListModel { parent }, m_selector(nullptr), m_valid(false), m_count(0)
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
    } else if (role == CanHaveRole) {
        return !s->canContain().isEmpty();
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
            beginInsertRows(QModelIndex(), 0, 0);
            setValid(validate());
            endInsertRows();
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
        if (s->has(type) && (type == "following" || type == "followers" || type == "me")) {
            // already exist
            return;
        }
        int s_child_count = s->getNodeCount();
        beginInsertRows(QModelIndex(), row + s_child_count, row + s_child_count);
        QJsonObject json;
        json.insert(type, QJsonObject());
        s->appendChildSelector(AbstractPostSelector::create(json, s));
        setValid(validate());
        endInsertRows();

        if (s->type() == "not") {
            emit dataChanged(index(row), index(row));
        }
    }
    setCount(rowCount());
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
        AbstractPostSelector *s_parent = m_selector->remove(s);
        endRemoveRows();

        if (s_parent != nullptr && s_parent->type() == "not") {
            int parent_row = m_selector->index(s_parent);
            if (parent_row >= 0 && parent_row < rowCount()) {
                emit dataChanged(index(parent_row), index(parent_row));
            }
        }
    }
    setValid(validate());
    setCount(rowCount());
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
    setCount(rowCount());
}

void EditSelectorListModel::save(const QString &uuid)
{
    AccountManager *manager = AccountManager::getInstance();
    manager->updateRealtimeFeedRule(uuid, name(), toJson());
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
    roles[CanHaveRole] = "canHave";
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

    clear();
    AbstractPostSelector *temp = AbstractPostSelector::create(json, this);
    if (temp != nullptr) {
        beginInsertRows(QModelIndex(), 0, temp->getNodeCount() - 1);
        m_selector = temp;
        endInsertRows();
    }
    setValid(validate());
    setCount(rowCount());
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

int EditSelectorListModel::count() const
{
    return m_count;
}

void EditSelectorListModel::setCount(int newCount)
{
    if (m_count == newCount)
        return;
    m_count = newCount;
    emit countChanged();
}

QString EditSelectorListModel::name() const
{
    return m_name;
}

void EditSelectorListModel::setName(const QString &newName)
{
    if (m_name == newName)
        return;
    m_name = newName;
    emit nameChanged();
}
