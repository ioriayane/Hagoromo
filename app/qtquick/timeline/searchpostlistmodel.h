#ifndef SEARCHPOSTLISTMODEL_H
#define SEARCHPOSTLISTMODEL_H

#include "timelinelistmodel.h"

struct SearchPostsV2Condition
{
    QString query;
    QString since;
    QString until;
    QStringList languages;
};

class SearchPostListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit SearchPostListModel(QObject *parent = nullptr);

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

    QString replaceSearchCommand(const QString &command);
    SearchPostsV2Condition makeSearchPostsV2Condition(const QString &command);

    QString text() const;
    void setText(const QString &newText);

signals:
    void textChanged();
    void searchServiceChanged();

private:
    void searchPostsV2Latest();
    void searchPostsV2Next();
    void searchPostsLatest();
    void searchPostsNext();
    bool isSearchPostsV2Unsupported(const int http_status_code, const QString &error_code) const;
    void switchToSearchPosts();
    void appendPosts(const QList<AtProtocolType::AppBskyFeedDefs::PostView> &posts,
                     const bool next);

    QStringList m_cueGetPost; // uri

    QString m_text;
    // searchPostsV2が未実装のサービスならsearchPostsに切り替えて以降は戻さない
    bool m_useSearchPostsV2;
};

#endif // SEARCHPOSTLISTMODEL_H
