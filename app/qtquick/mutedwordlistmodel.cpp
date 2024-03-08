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
        if (item.targets.contains(MutedWordTarget::Content)) {
            targets.append("content");
        }
        if (item.targets.contains(MutedWordTarget::Tag)) {
            targets.append("tag");
        }
        return targets;
    }

    return QVariant();
}

QHash<int, QByteArray> MutedWordListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[ValueRole] = "value";
    roles[TargetsRole] = "targets";

    return roles;
}
