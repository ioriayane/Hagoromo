#ifndef CUSTOMFEEDLISTMODEL_H
#define CUSTOMFEEDLISTMODEL_H

#include "timelinelistmodel.h"

class CustomFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)

public:
    explicit CustomFeedListModel(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();

    QString uri() const;
    void setUri(const QString &newUri);
signals:
    void uriChanged();

private:
    QString m_uri;
};

#endif // CUSTOMFEEDLISTMODEL_H
