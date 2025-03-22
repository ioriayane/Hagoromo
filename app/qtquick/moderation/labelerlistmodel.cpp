#include "labelerlistmodel.h"

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/labeler/appbskylabelergetservices.h"

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyLabelerGetServices;

#define BSKY_OFFICIAL_LABELER_DID QStringLiteral("did:plc:ar7c4by46qjdydhdevvrndac")

LabelerListModel::LabelerListModel(QObject *parent)
    : QAbstractListModel { parent }, m_running(false)
{
}

int LabelerListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_labelerList.count();
}

QVariant LabelerListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<LabelerListModelRoles>(role));
}

QVariant LabelerListModel::item(int row, LabelerListModelRoles role) const
{
    if (row < 0 || row >= m_labelerList.count())
        return QVariant();

    if (role == DidRole)
        return m_labelerList.at(row).creator.did;
    else if (role == TitleRole)
        return m_labelerList.at(row).creator.displayName;
    else if (role == DescriptionRole)
        return m_labelerList.at(row).creator.description;

    return QVariant();
}

void LabelerListModel::load()
{
    if (running())
        return;
    setRunning(true);

    AtProtocolInterface::AccountData account;

    account.service = service();
    account.handle = handle();
    account.accessJwt = accessJwt();

    if (!m_labelerList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_labelerList.count() - 1);
        m_labelerList.clear();
        endRemoveRows();
    }

    AppBskyActorGetPreferences *preferences = new AppBskyActorGetPreferences(this);
    connect(preferences, &AppBskyActorGetPreferences::finished, this, [=](bool success) {
        if (success) {
            QStringList dids;
            for (const auto &labeler_pref : preferences->preferences().labelersPref) {
                for (const auto &labeler : labeler_pref.labelers) {
                    dids.append(labeler.did);
                }
            }

            if (!dids.contains(BSKY_OFFICIAL_LABELER_DID)) {
                dids.insert(0, BSKY_OFFICIAL_LABELER_DID);
            }

            AppBskyLabelerGetServices *services = new AppBskyLabelerGetServices(this);
            connect(services, &AppBskyLabelerGetServices::finished, [=](bool success) {
                if (success) {
                    beginInsertRows(QModelIndex(), 0, services->viewsLabelerViewList().count() - 1);
                    m_labelerList = services->viewsLabelerViewList();
                    endInsertRows();
                    emit finished();
                } else {
                    emit errorOccured(services->errorCode(), services->errorMessage());
                }
                setRunning(false);
                services->deleteLater();
            });
            services->setAccount(account);
            services->getServices(dids, false);
        } else {
            emit errorOccured(preferences->errorCode(), preferences->errorMessage());
            setRunning(false);
        }
        preferences->deleteLater();
    });
    preferences->setAccount(account);
    preferences->getPreferences();
}

bool LabelerListModel::running() const
{
    return m_running;
}

void LabelerListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString LabelerListModel::service() const
{
    return m_service;
}

void LabelerListModel::setService(const QString &newService)
{
    if (m_service == newService)
        return;
    m_service = newService;
    emit serviceChanged();
}

QString LabelerListModel::handle() const
{
    return m_handle;
}

void LabelerListModel::setHandle(const QString &newHandle)
{
    if (m_handle == newHandle)
        return;
    m_handle = newHandle;
    emit handleChanged();
}

QString LabelerListModel::accessJwt() const
{
    return m_accessJwt;
}

void LabelerListModel::setAccessJwt(const QString &newAccessJwt)
{
    if (m_accessJwt == newAccessJwt)
        return;
    m_accessJwt = newAccessJwt;
    emit accessJwtChanged();
}

QHash<int, QByteArray> LabelerListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DidRole] = "did";
    roles[TitleRole] = "title";
    roles[DescriptionRole] = "description";

    return roles;
}
