#ifndef LISTFEEDLISTMODEL_H
#define LISTFEEDLISTMODEL_H

#include "timelinelistmodel.h"

class ListFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)

public:
    explicit ListFeedListModel(QObject *parent = nullptr);

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

    QString uri() const;
    void setUri(const QString &newUri);
signals:
    void uriChanged();

private:
    QString m_uri;
};

#endif // LISTFEEDLISTMODEL_H
