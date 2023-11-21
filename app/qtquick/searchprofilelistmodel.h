#ifndef SEARCHPROFILELISTMODEL_H
#define SEARCHPROFILELISTMODEL_H

#include "followslistmodel.h"

class SearchProfileListModel : public FollowsListModel
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit SearchProfileListModel(QObject *parent = nullptr);

    QString text() const;
    void setText(const QString &newText);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

signals:
    void textChanged();

private:
    QString m_text;
};

#endif // SEARCHPROFILELISTMODEL_H
