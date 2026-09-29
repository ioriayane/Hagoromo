#include "labelerlistmodel.h"

#include "atprotocol/accessatprotocol.h"
#include "tools/accountmanager.h"
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

    const AtProtocolInterface::AccountData account =
            AccountManager::getInstance()->getAccount(m_account);

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
            connect(services, &AppBskyLabelerGetServices::finished, this, [=](bool success) {
                if (success) {
                    beginInsertRows(QModelIndex(), 0, services->viewsLabelerViewList().count() - 1);
                    m_labelerList = services->viewsLabelerViewList();
                    endInsertRows();
                    emit finished();
                } else {
                    emit errorOccurred(services->errorCode(), services->errorMessage());
                }
                setRunning(false);
                services->deleteLater();
            });
            services->setAccount(account);
            services->getServices(dids, false);
        } else {
            emit errorOccurred(preferences->errorCode(), preferences->errorMessage());
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

QString LabelerListModel::account() const
{
    return m_account;
}

void LabelerListModel::setAccount(const QString &uuid)
{
    if (m_account == uuid)
        return;
    m_account = uuid;
    emit accountChanged();
}

QHash<int, QByteArray> LabelerListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DidRole] = "did";
    roles[TitleRole] = "title";
    roles[DescriptionRole] = "description";

    return roles;
}
