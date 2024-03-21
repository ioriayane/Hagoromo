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
            targets.append("text");
        }
        if (item.targets.contains(MutedWordTarget::Tag)) {
            targets.append("tag");
        }
        return targets;
    }

    return QVariant();
}

int MutedWordListModel::append(const QString &value, bool tag_only)
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
        row = rowCount();
        beginInsertRows(QModelIndex(), row, row);
        m_contentFilterLabels.insertMutedWord(row, value, targets);
        endInsertRows();
    }

    setModified(true);

    return row;
}

void MutedWordListModel::remove(int row)
{
    if (row < 0 || row >= rowCount())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_contentFilterLabels.removeMutedWordItem(row);
    endRemoveRows();

    setModified(true);
}

void MutedWordListModel::clear()
{
    if (m_contentFilterLabels.mutedWordCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_contentFilterLabels.mutedWordCount() - 1);
        m_contentFilterLabels.clearMutedWord();
        endRemoveRows();
    }
}

QHash<int, QByteArray> MutedWordListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[ValueRole] = "value";
    roles[TargetsRole] = "targets";

    return roles;
}

bool MutedWordListModel::modified() const
{
    return m_modified;
}

void MutedWordListModel::setModified(bool newModified)
{
    if (m_modified == newModified)
        return;
    m_modified = newModified;
    emit modifiedChanged();
}
