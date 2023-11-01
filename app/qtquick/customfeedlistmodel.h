#ifndef CUSTOMFEEDLISTMODEL_H
#define CUSTOMFEEDLISTMODEL_H

#include "timelinelistmodel.h"
#include "feedgeneratorlistmodel.h"

class CustomFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(bool saving READ saving WRITE setSaving NOTIFY savingChanged)

public:
    explicit CustomFeedListModel(QObject *parent = nullptr);

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE void updateFeedSaveStatus();
    Q_INVOKABLE void saveGenerator();
    Q_INVOKABLE void removeGenerator();
    Q_INVOKABLE QString getOfficialUrl() const;

    QString uri() const;
    void setUri(const QString &newUri);
    bool saving() const;
    void setSaving(bool newSaving);

signals:
    void uriChanged();

    void savingChanged();

private:
    FeedGeneratorListModel m_feedGeneratorListModel;
    QString m_uri;
    bool m_saving;
};

#endif // CUSTOMFEEDLISTMODEL_H
