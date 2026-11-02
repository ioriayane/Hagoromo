#include "blogentrylistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "extension/com/atproto/repo/comatprotorepolistrecordsex.h"
#include "tools/accountmanager.h"

using AtProtocolInterface::ComAtprotoRepoListRecordsEx;

BlogEntryListModel::BlogEntryListModel(QObject *parent)
    : QAbstractListModel { parent }, m_running(false)
{
    connect(&m_serviceEndpointResolver, &ServiceEndpointResolver::errorOccurred, this,
            &BlogEntryListModel::errorOccurred);
}

int BlogEntryListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_blogEntryRecordList.count();
}

QVariant BlogEntryListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<BlogEntryListModelRoles>(role));
}

QVariant BlogEntryListModel::item(int row, BlogEntryListModelRoles role) const
{
    if (row < 0 || row >= m_blogEntryRecordList.count())
        return QVariant();

    if (m_blogEntryRecordList.at(row).uri.split("/").contains("blue.linkat.board")) {
        return itemFromLinkat(row, role);
    } else {
        return itemFromWhiteWind(row, role);
    }

    return QVariant();
}

AtProtocolInterface::AccountData BlogEntryListModel::account() const
{
    return AccountManager::getInstance()->getAccount(m_account.uuid);
}

void BlogEntryListModel::setAccount(const QString &uuid)
{
    m_account.uuid = uuid;
}

bool BlogEntryListModel::getLatest()
{
    if (running() || targetDid().isEmpty())
        return false;
    setRunning(true);

    if (!m_blogEntryRecordList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_blogEntryRecordList.count() - 1);
        m_blogEntryRecordList.clear();
        endRemoveRows();
    }

    // 対象のアカウントのレコードは対象のPDSから取得する
    m_serviceEndpointResolver.resolve(account(), targetDid(), targetServiceEndpoint(),
                                      [=](bool success, const QString &service_endpoint) {
                                          if (!success) {
                                              setRunning(false);
                                              return;
                                          }
                                          getLatestFromLinkat(service_endpoint, [=](bool success) {
                                              if (success) {
                                                  getLatestFromWhiteWind(service_endpoint,
                                                                         [=](bool success) {
                                                                             Q_UNUSED(success)
                                                                             setRunning(false);
                                                                         });
                                              } else {
                                                  setRunning(false);
                                              }
                                          });
                                      });

    return true;
}

QHash<int, QByteArray> BlogEntryListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[CidRole] = "cid";
    roles[UriRole] = "uri";

    roles[ServiceNameRole] = "serviceName";
    roles[TitleRole] = "title";
    roles[ContentRole] = "content";
    roles[CreatedAtRole] = "createdAt";
    roles[VisibilityRole] = "visibility";
    roles[PermalinkRole] = "permalink";

    return roles;
}

QVariant BlogEntryListModel::itemFromWhiteWind(int row, BlogEntryListModelRoles role) const
{
    if (row < 0 || row >= m_blogEntryRecordList.count())
        return QVariant();

    const auto &current = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
            AtProtocolType::ComWhtwndBlogEntry::Main>(m_blogEntryRecordList.at(row).value);

    if (role == CidRole)
        return m_blogEntryRecordList.at(row).cid;
    else if (role == UriRole)
        return m_blogEntryRecordList.at(row).uri;
    else if (role == ServiceNameRole)
        return "WhiteWind";
    else if (role == TitleRole)
        return current.title;
    else if (role == ContentRole)
        return current.content;
    else if (role == CreatedAtRole)
        return AtProtocolType::LexiconsTypeUnknown::formatDateTime(current.createdAt);
    else if (role == VisibilityRole) {
        if (current.visibility == "public") {
            return QString();
        } else if (current.visibility == "url") {
            if (isOwnRecord(m_blogEntryRecordList.at(row).uri, account().did)) {
                return QStringLiteral("Anyone with the link");
            } else {
                return QString();
            }
        } else if (current.visibility == "author") {
            return QStringLiteral("Author only");
        } else {
            return current.visibility;
        }
    } else if (role == PermalinkRole) {
        QStringList items = m_blogEntryRecordList.at(row).uri.split("/");
        if (items.length() == 5) {
            return QString("https://whtwnd.com/%1/%2/%3")
                    .arg(items.at(2))
                    .arg(items.at(4))
                    .arg(m_blogEntryRecordList.at(row).cid);
        } else {
            return QStringLiteral("https://whtwnd.com");
        }
    }

    return QVariant();
}

bool BlogEntryListModel::isOwnRecord(const QString &uri, const QString &did)
{
    // at://did/collection/rkey
    // アカウントの情報が取れない(didが空)ときは自分のレコードとして扱わない
    const QStringList items = uri.split("/");
    return !did.isEmpty() && items.length() == 5 && items.at(2) == did;
}

QVariant BlogEntryListModel::itemFromLinkat(int row, BlogEntryListModelRoles role) const
{
    if (row < 0 || row >= m_blogEntryRecordList.count())
        return QVariant();

    const auto &current = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
            AtProtocolType::BlueLinkatBoard::Main>(m_blogEntryRecordList.at(row).value);

    if (current.cards.isEmpty())
        return QVariant();

    if (role == CidRole)
        return m_blogEntryRecordList.at(row).cid;
    else if (role == UriRole)
        return m_blogEntryRecordList.at(row).uri;
    else if (role == ServiceNameRole)
        return "Linkat";
    else if (role == TitleRole) {
        if (current.cards.first().text.isEmpty()) {
            return current.cards.first().url;
        } else {
            return current.cards.first().text;
        }
    } else if (role == ContentRole)
        if (current.cards.length() > 1) {
            return "and more ...";
        } else {
            return QString();
        }
    else if (role == CreatedAtRole)
        return QString();
    else if (role == VisibilityRole) {
        return QString();
    } else if (role == PermalinkRole) {
        return QStringLiteral("https://linkat.blue/") + targetHandle();
    }

    return QVariant();
}

void BlogEntryListModel::getLatestFromWhiteWind(const QString &service_endpoint,
                                                std::function<void(const bool &)> callback)
{
    ComAtprotoRepoListRecordsEx *record = new ComAtprotoRepoListRecordsEx(this);
    connect(record, &ComAtprotoRepoListRecordsEx::finished, this, [=](bool success) {
        if (success) {
            const QString did = account().did;
            for (const auto &r : record->recordsList()) {
                const auto &current = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                        AtProtocolType::ComWhtwndBlogEntry::Main>(r.value);
                if (current.visibility != "author" || isOwnRecord(r.uri, did)) {
                    beginInsertRows(QModelIndex(), rowCount(), rowCount());
                    m_blogEntryRecordList.append(r);
                    endInsertRows();
                    if (m_blogEntryRecordList.length() >= 5) {
                        break;
                    }
                }
            }
        } else {
            emit errorOccurred(record->errorCode(), record->errorMessage());
        }
        callback(success);
        record->deleteLater();
    });
    record->setAccount(account());
    if (!service_endpoint.isEmpty()) {
        record->setService(service_endpoint);
    }
    record->listWhiteWindItems(targetDid(), QString());
}

void BlogEntryListModel::getLatestFromLinkat(const QString &service_endpoint,
                                             std::function<void(const bool &)> callback)
{
    ComAtprotoRepoListRecordsEx *record = new ComAtprotoRepoListRecordsEx(this);
    connect(record, &ComAtprotoRepoListRecordsEx::finished, this, [=](bool success) {
        if (success) {
            for (const auto &r : record->recordsList()) {
                const auto &current = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                        AtProtocolType::BlueLinkatBoard::Main>(r.value);
                if (!current.cards.isEmpty()) {
                    beginInsertRows(QModelIndex(), rowCount(), rowCount());
                    m_blogEntryRecordList.append(r);
                    endInsertRows();
                    break;
                }
            }
        } else {
            emit errorOccurred(record->errorCode(), record->errorMessage());
        }
        callback(success);
        record->deleteLater();
    });
    record->setAccount(account());
    if (!service_endpoint.isEmpty()) {
        record->setService(service_endpoint);
    }
    record->listLinkatItems(targetDid(), QString());
}

bool BlogEntryListModel::running() const
{
    return m_running;
}

void BlogEntryListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString BlogEntryListModel::targetDid() const
{
    return m_targetDid;
}

void BlogEntryListModel::setTargetDid(const QString &newTargetDid)
{
    if (m_targetDid == newTargetDid)
        return;
    m_targetDid = newTargetDid;
    emit targetDidChanged();
}

QString BlogEntryListModel::targetServiceEndpoint() const
{
    return m_targetServiceEndpoint;
}

void BlogEntryListModel::setTargetServiceEndpoint(const QString &newTargetServiceEndpoint)
{
    if (m_targetServiceEndpoint == newTargetServiceEndpoint)
        return;
    m_targetServiceEndpoint = newTargetServiceEndpoint;
    emit targetServiceEndpointChanged();
}

QString BlogEntryListModel::targetHandle() const
{
    return m_targetHandle;
}

void BlogEntryListModel::setPlcDirectory(const QString &newPlcDirectory)
{
    m_serviceEndpointResolver.setPlcDirectory(newPlcDirectory);
}

void BlogEntryListModel::setTargetHandle(const QString &newTargetHandle)
{
    if (m_targetHandle == newTargetHandle)
        return;
    m_targetHandle = newTargetHandle;
    emit targetHandleChanged();
}
