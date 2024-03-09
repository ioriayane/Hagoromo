#include "mutedwordlistmodel.h"

MutedWordListModel::MutedWordListModel(QObject *parent) : ContentFilterSettingListModel { parent }
{
}

int MutedWordListModel::rowCount(const QModelIndex &parent) const
{
    return m_contentFilterLabels.mutedWordCount();
}

QVariant MutedWordListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<MutedWordListModelRoles>(role));
}

QVariant MutedWordListModel::item(int row, MutedWordListModelRoles role) const
{
    if (row < 0 || row >= m_contentFilterLabels.mutedWordCount())
        return QVariant();

    MutedWordItem item = m_contentFilterLabels.getMutedWordItem(row);
    if (role == ValueRole)
        return item.value;
    else if (role == TargetsRole) {
        QStringList targets;
        if (item.targets.contains(MutedWordTarget::Tag)) {
            targets.append("tag");
        }
        if (item.targets.contains(MutedWordTarget::Content)) {
            targets.append("text");
        }
        return targets;
    }

    return QVariant();
}

void MutedWordListModel::append(const QString &value, bool tag_only)
{
    QList<MutedWordTarget> targets;
    if (!tag_only) {
        targets.append(MutedWordTarget::Content);
    }
    targets.append(MutedWordTarget::Tag);

    int row = m_contentFilterLabels.indexOfMutedWordItem(value);
    if (row >= 0) {
        m_contentFilterLabels.updateMutedWord(row, value, targets);
        emit dataChanged(index(row), index(row));
    } else {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_contentFilterLabels.insertMutedWord(rowCount(), value, targets);
        endInsertRows();
    }
}

void MutedWordListModel::remove(int row)
{
    if (row < 0 || row >= rowCount())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_contentFilterLabels.removeMutedWordItem(row);
    endRemoveRows();
}

QHash<int, QByteArray> MutedWordListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[ValueRole] = "value";
    roles[TargetsRole] = "targets";

    return roles;
}
