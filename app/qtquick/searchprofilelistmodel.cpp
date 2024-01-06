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

QString SearchProfileListModel::extractHandleBlock(const QString &text)
{
    int pos = text.lastIndexOf("@");
    if (pos >= 0) {
        return m_regMentionHandle.match(text, pos + 1).captured();
    }
    return QString();
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
