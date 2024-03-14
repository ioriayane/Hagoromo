#include "contentfiltersettinglistmodel.h"

ContentFilterSettingListModel::ContentFilterSettingListModel(QObject *parent)
    : QAbstractListModel { parent }, m_saving(false), m_running(false)
{
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
                    beginInsertRows(QModelIndex(), 0, m_contentFilterLabels.count() - 1);
                    endInsertRows();
                }
                emit enableAdultContentChanged();
            }
        }
        m_saving = false;
        emit finished();
    });
}

int ContentFilterSettingListModel::rowCount(const QModelIndex &parent) const
{
    return m_contentFilterLabels.count();
}

QVariant ContentFilterSettingListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ContentFilterSettingListModelRoles>(role));
}

QVariant ContentFilterSettingListModel::item(int row, ContentFilterSettingListModelRoles role) const
{
    if (row < 0 || row >= m_contentFilterLabels.count())
        return QVariant();

    if (role == TitleRole)
        return m_contentFilterLabels.title(row);
    else if (role == DescriptionRole)
        return m_contentFilterLabels.description(row);
    else if (role == StatusRole) {
        if (m_contentFilterLabels.status(row) == ConfigurableLabelStatus::Hide) {
            return 0;
        } else if (m_contentFilterLabels.status(row) == ConfigurableLabelStatus::Warning) {
            return 1;
        } else if (m_contentFilterLabels.status(row) == ConfigurableLabelStatus::Show) {
            return 2;
        }
    } else if (role == IsAdultImageryRole)
        return m_contentFilterLabels.isAdultImagery(row);
    else if (role == ConfigurableRole)
        return m_contentFilterLabels.configurable(row);

    return QVariant();
}

void ContentFilterSettingListModel::update(int row, ContentFilterSettingListModelRoles role,
                                           const QVariant &value)
{
    if (row < 0 || row >= m_contentFilterLabels.count())
        return;

    if (role == TitleRole)
        m_contentFilterLabels.title(row);
    else if (role == DescriptionRole)
        m_contentFilterLabels.description(row);
    else if (role == StatusRole) {
        if (value.toInt() == 0) {
            m_contentFilterLabels.setStatus(row, ConfigurableLabelStatus::Hide);
        } else if (value.toInt() == 1) {
            m_contentFilterLabels.setStatus(row, ConfigurableLabelStatus::Warning);
        } else if (value.toInt() == 2) {
            m_contentFilterLabels.setStatus(row, ConfigurableLabelStatus::Show);
        }
    }
    emit dataChanged(index(row), index(row));
}

void ContentFilterSettingListModel::load()
{
    if (running())
        return;

    m_saving = false;
    if (m_contentFilterLabels.mutedWordCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_contentFilterLabels.mutedWordCount() - 1);
        endRemoveRows();
    }
    if (m_contentFilterLabels.count()) {
        beginRemoveRows(QModelIndex(), 0, m_contentFilterLabels.count() - 1);
        endRemoveRows();
    }

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

QHash<int, QByteArray> ContentFilterSettingListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[TitleRole] = "title";
    roles[DescriptionRole] = "description";
    roles[StatusRole] = "status";
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
