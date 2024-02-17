#ifndef LISTLINK_H
#define LISTLINK_H

#include "feedgeneratorlink.h"

class ListLink : public FeedGeneratorLink
{
    Q_OBJECT

    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)

public:
    explicit ListLink(QObject *parent = nullptr);

    Q_INVOKABLE void getList(const QString &uri);
    Q_INVOKABLE void clear();
    void getFeedGenerator(const QString &uri) = delete;

    QString description() const;
    void setDescription(const QString &newDescription);
signals:
    void descriptionChanged();

private:
    QString m_description;
};

#endif // LISTLINK_H
