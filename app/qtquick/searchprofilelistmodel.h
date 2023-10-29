#ifndef SEARCHPROFILELISTMODEL_H
#define SEARCHPROFILELISTMODEL_H

#include "followslistmodel.h"

class SearchProfileListModel : public FollowsListModel
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString searchService READ searchService WRITE setSearchService NOTIFY
                       searchServiceChanged)
public:
    explicit SearchProfileListModel(QObject *parent = nullptr);

    QString text() const;
    void setText(const QString &newText);
    QString searchService() const;
    void setSearchService(const QString &newSearchService);

public slots:
    Q_INVOKABLE bool getLatest();

signals:
    void textChanged();
    void searchServiceChanged();

private:
    QString m_text;
    QString m_searchService;
};

#endif // SEARCHPROFILELISTMODEL_H
