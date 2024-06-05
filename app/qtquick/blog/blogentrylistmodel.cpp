#include "blogentrylistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "extension/com/atproto/repo/comatprotorepolistrecordsex.h"

using AtProtocolInterface::ComAtprotoRepoListRecordsEx;

BlogEntryListModel::BlogEntryListModel(QObject *parent)
    : QAbstractListModel { parent }, m_running(false)
{
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
            if (m_blogEntryRecordList.at(row).uri.split("/").contains(account().did)) {
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

AtProtocolInterface::AccountData BlogEntryListModel::account() const
{
    return m_account;
}

void BlogEntryListModel::setAccount(const QString &service, const QString &did,
                                    const QString &handle, const QString &email,
                                    const QString &accessJwt, const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
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

    ComAtprotoRepoListRecordsEx *record = new ComAtprotoRepoListRecordsEx(this);
    connect(record, &ComAtprotoRepoListRecordsEx::finished, this, [=](bool success) {
        if (success) {
            for (const auto &r : record->recordsList()) {
                const auto &current = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                        AtProtocolType::ComWhtwndBlogEntry::Main>(r.value);
                if (current.visibility != "author" || r.uri.split("/").contains(account().did)) {
                    beginInsertRows(QModelIndex(), rowCount(), rowCount());
                    m_blogEntryRecordList.append(r);
                    endInsertRows();
                    if (m_blogEntryRecordList.length() >= 5) {
                        break;
                    }
                }
            }
        } else {
            emit errorOccured(record->errorCode(), record->errorMessage());
        }
        setRunning(false);
        record->deleteLater();
    });
    record->setAccount(account());
    if (!targetServiceEndpoint().isEmpty()) {
        record->setService(targetServiceEndpoint());
    }
    record->listWhiteWindItems(targetDid(), QString());

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
