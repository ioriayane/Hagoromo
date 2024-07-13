#ifndef SEARCHPOSTLISTMODEL_H
#define SEARCHPOSTLISTMODEL_H

#include "timelinelistmodel.h"

class SearchPostListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit SearchPostListModel(QObject *parent = nullptr);

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

    QString replaceSearchCommand(const QString &command);

    QString text() const;
    void setText(const QString &newText);

signals:
    void textChanged();
    void searchServiceChanged();

private:
    QStringList m_cueGetPost; // uri

    QString m_text;
};

#endif // SEARCHPOSTLISTMODEL_H
