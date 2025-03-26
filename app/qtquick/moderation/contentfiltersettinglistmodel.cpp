#include "contentfiltersettinglistmodel.h"

ContentFilterSettingListModel::ContentFilterSettingListModel(QObject *parent)
    : QAbstractListModel { parent }, m_saving(false), m_running(false), m_modified(false)
{
    m_contentFilterLabels.setRefreshLabelers(true);

    connect(&m_contentFilterLabels, &ConfigurableLabels::runningChanged, this,
            &ContentFilterSettingListModel::setRunning);
    connect(&m_contentFilterLabels, &ConfigurableLabels::finished, this, [=](bool success) {
        if (success) {
            if (strcmp(this->metaObject()->className(), "MutedWordListModel") == 0) {
                if (m_contentFilterLabels.mutedWordCount() == 0) {
                } else if (m_saving) {
                    emit dataChanged(index(0), index(m_contentFilterLabels.mutedWordCount() - 1));
                } else {
                    // ミュートワードは0個スタートなのでinsert
                    // 再読み込み時もリセットされる
                    beginInsertRows(QModelIndex(), 0, m_contentFilterLabels.mutedWordCount() - 1);
                    endInsertRows();
                }
            } else {
                if (m_contentFilterLabels.count() == 0) {
                } else if (m_saving) {
                    emit dataChanged(index(0), index(rowCount() - 1));
                } else {
                    beginInsertRows(QModelIndex(), 0,
                                    m_contentFilterLabels.count(labelerDid()) - 1);
                    endInsertRows();
                }
                setSelectableLabelerDids(m_contentFilterLabels.labelerDids());
                emit enableAdultContentChanged();
            }
            setModified(false);
        }
        m_saving = false;
        emit finished();
    });
}

int ContentFilterSettingListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_contentFilterLabels.count(labelerDid());
}

QVariant ContentFilterSettingListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ContentFilterSettingListModelRoles>(role));
}

QVariant ContentFilterSettingListModel::item(int row, ContentFilterSettingListModelRoles role) const
{
    if (row < 0 || row >= m_contentFilterLabels.count(labelerDid()))
        return QVariant();

    if (role == TitleRole)
        return m_contentFilterLabels.title(row, labelerDid());
    else if (role == DescriptionRole)
        return m_contentFilterLabels.description(row, labelerDid());
    else if (role == StatusRole) {
        ConfigurableLabelStatus status = m_contentFilterLabels.status(row, labelerDid());
        if (status == ConfigurableLabelStatus::Hide) {
            return 0;
        } else if (status == ConfigurableLabelStatus::Warning) {
            return 1;
        } else if (status == ConfigurableLabelStatus::Show) {
            return 2;
        }
    } else if (role == LevelRole) {
        ConfigurableLabelLevel level = m_contentFilterLabels.level(row, labelerDid());
        if (level == ConfigurableLabelLevel::Alert) {
            return 0;
        } else if (level == ConfigurableLabelLevel::Inform) {
            return 1;
        } else {
            return 2;
        }
    } else if (role == IsAdultImageryRole) {
        return m_contentFilterLabels.isAdultImagery(row, labelerDid());
    } else if (role == ConfigurableRole) {
        return m_contentFilterLabels.configurable(row, labelerDid());
    }

    return QVariant();
}

void ContentFilterSettingListModel::update(int row, ContentFilterSettingListModelRoles role,
                                           const QVariant &value)
{
    if (row < 0 || row >= m_contentFilterLabels.count(labelerDid()))
        return;

    if (role == TitleRole)
        m_contentFilterLabels.title(row, labelerDid());
    else if (role == DescriptionRole)
        m_contentFilterLabels.description(row, labelerDid());
    else if (role == StatusRole) {
        if (value.toInt() == 0) {
            m_contentFilterLabels.setStatus(row, ConfigurableLabelStatus::Hide, labelerDid());
        } else if (value.toInt() == 1) {
            m_contentFilterLabels.setStatus(row, ConfigurableLabelStatus::Warning, labelerDid());
        } else if (value.toInt() == 2) {
            m_contentFilterLabels.setStatus(row, ConfigurableLabelStatus::Show, labelerDid());
        }
        setModified(true);
    }
    emit dataChanged(index(row), index(row));
}

void ContentFilterSettingListModel::load()
{
    if (running())
        return;

    m_saving = false;
    clear();

    m_contentFilterLabels.setService(service());
    m_contentFilterLabels.setSession(QString(), handle(), QString(), accessJwt(), QString());
    m_contentFilterLabels.load();
}

void ContentFilterSettingListModel::save()
{
    if (running())
        return;

    m_saving = true;
    m_contentFilterLabels.setService(service());
    m_contentFilterLabels.setSession(QString(), handle(), QString(), accessJwt(), QString());
    m_contentFilterLabels.save();
}

void ContentFilterSettingListModel::clear()
{
    if (m_contentFilterLabels.mutedWordCount() > 0) {
        if (strcmp(this->metaObject()->className(), "MutedWordListModel") == 0) {
            beginRemoveRows(QModelIndex(), 0, m_contentFilterLabels.mutedWordCount() - 1);
            endRemoveRows();
        }
    }
    if (m_contentFilterLabels.count() > 0) {
        if (strcmp(this->metaObject()->className(), "MutedWordListModel") != 0) {
            beginRemoveRows(QModelIndex(), 0, m_contentFilterLabels.count(labelerDid()) - 1);
            endRemoveRows();
        }
    }
}

QString ContentFilterSettingListModel::selectableLabelerName(const QString &did) const
{
    QString display_name = m_contentFilterLabels.labelerDisplayName(did);
    QString handle = m_contentFilterLabels.labelerHandle(did);
    return display_name.isEmpty() ? handle : display_name;
}

QString ContentFilterSettingListModel::selectableLabelerDescription(const QString &did) const
{
    return m_contentFilterLabels.labelerDescription(did);
}

QHash<int, QByteArray> ContentFilterSettingListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[TitleRole] = "title";
    roles[DescriptionRole] = "description";
    roles[StatusRole] = "status";
    roles[LevelRole] = "level";
    roles[IsAdultImageryRole] = "isAdultImagery";
    roles[ConfigurableRole] = "configurable";

    return roles;
}

bool ContentFilterSettingListModel::enableAdultContent() const
{
    return m_contentFilterLabels.enableAdultContent();
}

void ContentFilterSettingListModel::setEnableAdultContent(bool newEnableAdultContent)
{
    if (m_contentFilterLabels.enableAdultContent() == newEnableAdultContent)
        return;
    m_contentFilterLabels.setEnableAdultContent(newEnableAdultContent);
    emit enableAdultContentChanged();

    setModified(true);
}

bool ContentFilterSettingListModel::running() const
{
    return m_running;
}

void ContentFilterSettingListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString ContentFilterSettingListModel::service() const
{
    return m_service;
}

void ContentFilterSettingListModel::setService(const QString &newService)
{
    if (m_service == newService)
        return;
    m_service = newService;
    emit serviceChanged();
}

QString ContentFilterSettingListModel::handle() const
{
    return m_handle;
}

void ContentFilterSettingListModel::setHandle(const QString &newHandle)
{
    if (m_handle == newHandle)
        return;
    m_handle = newHandle;
    emit handleChanged();
}

QString ContentFilterSettingListModel::accessJwt() const
{
    return m_accessJwt;
}

void ContentFilterSettingListModel::setAccessJwt(const QString &newAccessJwt)
{
    if (m_accessJwt == newAccessJwt)
        return;
    m_accessJwt = newAccessJwt;
    emit accessJwtChanged();
}

QString ContentFilterSettingListModel::labelerDid() const
{
    return m_labelerDid;
}

void ContentFilterSettingListModel::setLabelerDid(const QString &newLabelerDid)
{
    if (m_labelerDid == newLabelerDid)
        return;

    bool is = (strcmp(this->metaObject()->className(), "MutedWordListModel") != 0);

    if (is) {
        int old_count = m_contentFilterLabels.count(m_labelerDid);
        int new_count = m_contentFilterLabels.count(newLabelerDid);
        qDebug() << "setLabelerDid" << old_count << new_count;
        if (old_count == new_count) {
            //
            m_labelerDid = newLabelerDid;
            emit dataChanged(index(0), index(old_count - 1));
        } else if (old_count < new_count) {
            // 増える
            beginInsertRows(QModelIndex(), old_count, new_count - 1);
            m_labelerDid = newLabelerDid;
            endInsertRows();
            if (old_count > 0) {
                emit dataChanged(index(0), index(old_count - 1));
            }
        } else {
            // 減る
            beginRemoveRows(QModelIndex(), new_count, old_count - 1);
            m_labelerDid = newLabelerDid;
            endRemoveRows();
            if (new_count > 0) {
                emit dataChanged(index(0), index(new_count - 1));
            }
        }
        setLabelerHasAdultOnly(m_contentFilterLabels.hasAdultOnly(m_labelerDid));
    } else {
        qDebug() << "setLabelerDid" << is;
        m_labelerDid = newLabelerDid;
    }

    emit labelerDidChanged();
}

QStringList ContentFilterSettingListModel::selectableLabelerDids() const
{
    return m_selectableLabelerDids;
}

void ContentFilterSettingListModel::setSelectableLabelerDids(
        const QStringList &newSelectableLabelerDids)
{
    if (m_selectableLabelerDids == newSelectableLabelerDids)
        return;
    m_selectableLabelerDids = newSelectableLabelerDids;
    emit selectableLabelerDidsChanged();
}

bool ContentFilterSettingListModel::labelerHasAdultOnly() const
{
    return m_labelerHasAdultOnly;
}

void ContentFilterSettingListModel::setLabelerHasAdultOnly(bool newLabelerHasAdultOnly)
{
    if (m_labelerHasAdultOnly == newLabelerHasAdultOnly)
        return;
    m_labelerHasAdultOnly = newLabelerHasAdultOnly;
    emit labelerHasAdultOnlyChanged();
}

bool ContentFilterSettingListModel::modified() const
{
    return m_modified;
}

void ContentFilterSettingListModel::setModified(bool newModified)
{
    if (m_modified == newModified)
        return;
    m_modified = newModified;
    emit modifiedChanged();
}
