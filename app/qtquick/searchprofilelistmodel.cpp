#include "searchprofilelistmodel.h"

#include "atprotocol/app/bsky/actor/appbskyactorsearchactors.h"
#include "atprotocol/app/bsky/actor/appbskyactorsearchactorstypeahead.h"

using AtProtocolInterface::AppBskyActorSearchActors;
using AtProtocolInterface::AppBskyActorSearchActorsTypeahead;

SearchProfileListModel::SearchProfileListModel(QObject *parent) : FollowsListModel { parent }
{
    m_regMentionHandle = QRegularExpression(REG_EXP_MENTION_PART);
}

bool SearchProfileListModel::getSuggestion(const QString &q, int limit)
{
    if (q.isEmpty())
        return false;

    AppBskyActorSearchActorsTypeahead *profiles = new AppBskyActorSearchActorsTypeahead(this);
    connect(profiles, &AppBskyActorSearchActorsTypeahead::finished, [=](bool success) {
        if (success) {
            clear();
            copyProfiles(profiles);
        } else {
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
        }
        setRunning(false);
        profiles->deleteLater();
    });
    profiles->setAccount(account());
    setRunning(profiles->searchActorsTypeahead(QString(), q, limit));
    if (!running()) {
        emit errorOccured(profiles->errorCode(), profiles->errorMessage());
    }
    return running();
}

QString SearchProfileListModel::extractHandleBlock(const QString &text) const
{
    QString part = text.split("\n").last();
    QString result = m_regMentionHandle.match(part).captured();
    if (!result.isEmpty()) {
        return result.remove(0, 1);
    }
    return result;
}

QString SearchProfileListModel::replaceText(const QString &text, const int current_position,
                                            const QString &handle) const
{
    if (current_position < 0 || handle.isEmpty())
        return text;

    QString left = text.left(current_position);
    QString right = text.right(text.count() - current_position);
    QString extract = extractHandleBlock(left);
    QString space;
    if (right.isEmpty()) {
        space = QString(" ");
    } else if (!right.front().isSpace()) {
        space = QString(" ");
    }

    if (extract.isEmpty()) {
        // 入力状況が候補を検索したときと変わっている
        // ただし空の時だけ、ハンドルのインクリメンタルサーチではないので抽出候補とハンドルは一致するとは限らない
        return text;
    }

    return QString("%1%2%3%4")
            .arg(left.left(left.count() - extract.length()), handle, space, right);
}

bool SearchProfileListModel::getLatest()
{
    if (running() || text().isEmpty())
        return false;
    setRunning(true);

    return updateContentFilterLabels([=]() {
        AppBskyActorSearchActors *profiles = new AppBskyActorSearchActors(this);
        connect(profiles, &AppBskyActorSearchActors::finished, [=](bool success) {
            if (success) {
                if (m_didList.isEmpty()) {
                    m_cursor = profiles->cursor();
                }
                copyProfiles(profiles);
            } else {
                emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            }
            setRunning(false);
            profiles->deleteLater();
        });
        profiles->setAccount(account());
        if (!profiles->searchActors(text(), 50, QString())) {
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            setRunning(false);
        }
    });
}

bool SearchProfileListModel::getNext()
{
    if (running() || text().isEmpty() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    return updateContentFilterLabels([=]() {
        AppBskyActorSearchActors *profiles = new AppBskyActorSearchActors(this);
        connect(profiles, &AppBskyActorSearchActors::finished, [=](bool success) {
            if (success) {
                m_cursor = profiles->cursor();
                copyProfiles(profiles);
            } else {
                emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            }
            setRunning(false);
            profiles->deleteLater();
        });
        profiles->setAccount(account());
        if (!profiles->searchActors(text(), 50, m_cursor)) {
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            setRunning(false);
        }
    });
}

QString SearchProfileListModel::text() const
{
    return m_text;
}

void SearchProfileListModel::setText(const QString &newText)
{
    if (m_text == newText)
        return;
    m_text = newText;
    emit textChanged();
}
