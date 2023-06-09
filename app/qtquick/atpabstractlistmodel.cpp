#include "atpabstractlistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "translator.h"

#include <QDesktopServices>
#include <QPointer>
#include <QUrlQuery>

using namespace AtProtocolType;

AtpAbstractListModel::AtpAbstractListModel(QObject *parent)
    : QAbstractListModel { parent },
      m_running(false),
      m_loadingInterval(5 * 60 * 1000),
      m_displayInterval(400)
{
    connect(&m_timer, &QTimer::timeout, this, &AtpAbstractListModel::getLatest);
}

void AtpAbstractListModel::clear()
{
    if (!m_cidList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_cidList.count() - 1);
        m_cidList.clear();
        endRemoveRows();
    }
    m_originalCidList.clear();
    m_translations.clear();
}

AtProtocolInterface::AccountData AtpAbstractListModel::account() const
{
    return m_account;
}

void AtpAbstractListModel::setAccount(const QString &service, const QString &did,
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

void AtpAbstractListModel::translate(const QString &cid)
{
    // indexで指定しないと同じcidが複数含まれる場合に正しく対応できない
    // indexにすると処理を始めてから追加の読み込みがあるとズレる

    QString record_text = getRecordText(cid);
    if (record_text.isEmpty())
        return;
    int row = indexOf(cid);
    if (row == -1)
        return;

    QPointer<AtpAbstractListModel> aliving(this);

    Translator *translator = new Translator();
    if (!translator->validSettings()) {
        // 設定画無いときはGoogle翻訳へ飛ばす
        QUrl url("https://translate.google.com/");
        QUrlQuery query;
        query.addQueryItem("sl", "auto");
        if (!translator->targetLanguage().isEmpty()) {
            query.addQueryItem("tl", translator->targetLanguage().toLower());
        }
        query.addQueryItem("text", record_text);
        url.setQuery(query);
        QDesktopServices::openUrl(url);
    } else {
        connect(translator, &Translator::finished, [=](const QString text) {
            if (aliving) {
                if (!text.isEmpty()) {
                    m_translations[cid] = text;
                    emit dataChanged(index(row), index(row));
                }
            }
            translator->deleteLater();
        });
        m_translations[cid] = "Now translating ...";
        emit dataChanged(index(row), index(row));
        translator->translate(record_text);
    }
}

void AtpAbstractListModel::reflectVisibility()
{
    int prev_row = -1;
    for (const auto &cid : qAsConst(m_originalCidList)) {
        if (checkVisibility(cid)) {
            // 表示させる
            if (m_cidList.contains(cid)) {
                prev_row = m_cidList.indexOf(cid);
            } else {
                prev_row++;
                beginInsertRows(QModelIndex(), prev_row, prev_row);
                m_cidList.insert(prev_row, cid);
                endInsertRows();
            }
        } else {
            // 消す
            if (m_cidList.contains(cid)) {
                int r = m_cidList.indexOf(cid);
                beginRemoveRows(QModelIndex(), r, r);
                m_cidList.removeAt(r);
                endRemoveRows();
                prev_row = r - 1;
            }
        }
    }
}

bool AtpAbstractListModel::running() const
{
    return m_running;
}

void AtpAbstractListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString AtpAbstractListModel::formatDateTime(const QString &value) const
{
    return QDateTime::fromString(value, Qt::ISODateWithMs).toLocalTime().toString("MM/dd hh:mm");
}

QString AtpAbstractListModel::copyRecordText(const QVariant &value) const
{
    const AppBskyFeedPost::Main record =
            LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(value);
    if (record.facets.isEmpty()) {
        return QString(record.text).replace("\n", "<br/>");
    } else {
        QByteArray text_ba = record.text.toUtf8();
        QString text;
        int pos_start = 0;
        int pos_end = 0;
        int pos_prev_end = 0;
        QList<AppBskyRichtextFacet::Main> facets = record.facets;
        for (int i = 0; i < facets.length() - 1; i++) {
            for (int j = i + 1; j < facets.length(); j++) {
                if (facets.at(i).index.byteStart > facets.at(j).index.byteStart) {
                    facets.swapItemsAt(i, j);
                }
            }
        }
        for (const auto &part : facets) {
            pos_start = part.index.byteStart;
            pos_end = part.index.byteEnd;

            if (pos_start > pos_prev_end) {
                text += QString(text_ba.mid(pos_prev_end, pos_start - pos_prev_end))
                                .replace("\n", "<br/>");
            }
            QString display_url = QString::fromUtf8(text_ba.mid(pos_start, pos_end - pos_start));
            if (!part.features_Link.isEmpty()) {
                text += QString("<a href=\"%1\">%2</a>")
                                .arg(part.features_Link.first().uri, display_url);
            } else if (!part.features_Mention.isEmpty()) {
                text += QString("<a href=\"%1\">%2</a>")
                                .arg(part.features_Mention.first().did, display_url);
            } else {
                text += QString(text_ba.mid(pos_start, pos_end - pos_start)).replace("\n", "<br/>");
            }
            pos_prev_end = pos_end;
        }
        if (pos_prev_end < (text_ba.length() - 1)) {
            text += QString(text_ba.mid(pos_prev_end)).replace("\n", "<br/>");
        }

        return text;
    }
}

void AtpAbstractListModel::displayQueuedPosts()
{
    int interval = m_displayInterval;

    if (!m_cuePost.isEmpty()) {
        const PostCueItem &post = m_cuePost.front();
        bool visible = checkVisibility(post.cid);
        if (!visible)
            interval = 0;

        if (m_originalCidList.contains(post.cid)) {
            if (post.reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost
                && (QDateTime::fromString(post.indexed_at, Qt::ISODateWithMs)
                    > post.reference_time)) {
                // repostのときはいったん消す（上に持っていく）
                int r;
                if (visible) {
                    r = m_cidList.indexOf(post.cid);
                    if (r > 0) {
                        beginMoveRows(QModelIndex(), r, r, QModelIndex(), 0);
                        m_cidList.move(r, 0);
                        endMoveRows();
                    }
                }
                r = m_originalCidList.indexOf(post.cid);
                if (r > 0) {
                    m_originalCidList.move(r, 0);
                }
            } else {
                // リストは更新しないでデータのみ入れ替える
                // 更新をUIに通知
                // （取得できた範囲でしか更新できないのだけど・・・）
                interval = 0;
                int r = m_cidList.indexOf(post.cid);
                if (r > 0) {
                    emit dataChanged(index(r), index(r));
                }
            }
        } else {
            if (visible) {
                beginInsertRows(QModelIndex(), 0, 0);
                m_cidList.insert(0, post.cid);
                endInsertRows();
            }
            m_originalCidList.insert(0, post.cid);
        }

        m_cuePost.pop_front();
    }

    if (!m_cuePost.isEmpty()) {
        QTimer::singleShot(interval, this, &AtpAbstractListModel::displayQueuedPosts);
    } else {
        finishedDisplayingQueuedPosts();
    }
}

int AtpAbstractListModel::displayInterval() const
{
    return m_displayInterval;
}

void AtpAbstractListModel::setDisplayInterval(int newDisplayInterval)
{
    if (m_displayInterval == newDisplayInterval)
        return;
    m_displayInterval = newDisplayInterval;
    emit displayIntervalChanged();
}

bool AtpAbstractListModel::autoLoading() const
{
    return m_timer.isActive();
}

void AtpAbstractListModel::setAutoLoading(bool newAutoLoading)
{
    if (newAutoLoading) {
        // Off -> On
        if (m_timer.isActive())
            return;
        m_timer.start(m_loadingInterval);
    } else {
        // * -> Off
        m_timer.stop();
    }
    emit autoLoadingChanged();
}

int AtpAbstractListModel::loadingInterval() const
{
    return m_loadingInterval;
}

void AtpAbstractListModel::setLoadingInterval(int newLoadingInterval)
{
    if (m_loadingInterval == newLoadingInterval)
        return;
    m_loadingInterval = newLoadingInterval;
    m_timer.setInterval(m_loadingInterval);
    emit loadingIntervalChanged();
}

QString AtpAbstractListModel::service() const
{
    return account().service;
}

QString AtpAbstractListModel::did() const
{
    return account().did;
}

QString AtpAbstractListModel::handle() const
{
    return account().handle;
}

QString AtpAbstractListModel::email() const
{
    return account().email;
}

QString AtpAbstractListModel::accessJwt() const
{
    return account().accessJwt;
}

QString AtpAbstractListModel::refreshJwt() const
{
    return account().refreshJwt;
}
