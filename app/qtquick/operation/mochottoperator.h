#ifndef MOCHOTTOPERATOR_H
#define MOCHOTTOPERATOR_H

#include "atprotocol/lexicons.h"
#include "tools/mochottlongpost.h"

#include <QCache>
#include <QObject>
#include <QSet>
#include <QString>

class MochottOperator : public QObject
{
    Q_OBJECT
public:
    explicit MochottOperator(QObject *parent = nullptr);

    static MochottOperator *getInstance();

    void setServiceUrl(const QString &url);

    // cidはポストのCID
    void getArticle(const QString &cid, const MochottLongPost::Reference &reference,
                    const AtProtocolType::AppBskyFeedPost::Main &record, const QString &author_did);

    bool running(const QString &cid) const;
    bool failed(const QString &cid) const;
    // 取得できていなければvalidがfalseのものを返す
    MochottLongPost::Article article(const QString &cid) const;

signals:
    void stateChanged(const QString &cid);

private:
    QString m_serviceUrl;
    QSet<QString> m_running;
    QSet<QString> m_failed;
    QCache<QString, MochottLongPost::Article> m_articles; // QCache<cid, 記事>
};

#endif // MOCHOTTOPERATOR_H
