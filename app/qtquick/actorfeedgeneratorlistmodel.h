#ifndef ACTORFEEDGENERATORLISTMODEL_H
#define ACTORFEEDGENERATORLISTMODEL_H

#include "feedgeneratorlistmodel.h"

class ActorFeedGeneratorListModel : public FeedGeneratorListModel
{
    Q_OBJECT

    Q_PROPERTY(QString actor READ actor WRITE setActor NOTIFY actorChanged)
public:
    explicit ActorFeedGeneratorListModel(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void getNext();

    QString actor() const;
    void setActor(const QString &newActor);

signals:
    void actorChanged();

private:
    QString m_actor;
};

#endif // ACTORFEEDGENERATORLISTMODEL_H
