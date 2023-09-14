#ifndef ACTORLIKELISTMODEL_H
#define ACTORLIKELISTMODEL_H

#include "timelinelistmodel.h"

// 認証ユーザー以外の情報はとれないので未使用

class ActorLikeListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString actor READ actor WRITE setActor NOTIFY actorChanged)
public:
    explicit ActorLikeListModel(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void getNext();

    QString actor() const;
    void setActor(const QString &newActor);

signals:
    void actorChanged();

private:
    QString m_actor;
};

#endif // ACTORLIKELISTMODEL_H
