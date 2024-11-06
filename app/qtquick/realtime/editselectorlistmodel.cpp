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
    } else if (role == IndentRole) {
        index = row;
        return m_selector->indentAt(index, 0);
    }
    return QVariant();
}

QHash<int, QByteArray> EditSelectorListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[TypeRole] = "type";
    roles[DisplayTypeRole] = "displayType";
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
