#ifndef POSTLINK_H
#define POSTLINK_H

#include "feedgeneratorlink.h"

class PostLink : public FeedGeneratorLink
{
    Q_OBJECT

    Q_PROPERTY(QString indexedAt READ indexedAt WRITE setIndexedAt NOTIFY indexedAtChanged FINAL)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)

public:
    explicit PostLink(QObject *parent = nullptr);

    Q_INVOKABLE void getPost(const QString &uri);
    Q_INVOKABLE void clear();
    void getFeedGenerator(const QString &uri) = delete;

    QString indexedAt() const;
    void setIndexedAt(const QString &newIndexedAt);
    QString text() const;
    void setText(const QString &newText);

signals:
    void indexedAtChanged();
    void textChanged();

private:
    QString m_indexedAt;
    QString m_text;
};

#endif // POSTLINK_H
