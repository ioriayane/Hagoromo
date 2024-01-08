#ifndef SEARCHPROFILELISTMODEL_H
#define SEARCHPROFILELISTMODEL_H

#include "followslistmodel.h"

class SearchProfileListModel : public FollowsListModel
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    explicit SearchProfileListModel(QObject *parent = nullptr);

    Q_INVOKABLE bool getSuggestion(const QString &q, int limit);
    Q_INVOKABLE QString extractHandleBlock(const QString &text) const;
    Q_INVOKABLE QString replaceText(const QString &text, const int current_position,
                                    const QString &handle) const;

    QString text() const;
    void setText(const QString &newText);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

signals:
    void textChanged();

private:
    QString m_text;
    QRegularExpression m_regMentionHandle;
};

#endif // SEARCHPROFILELISTMODEL_H
