#ifndef ANYPROFILELISTMODEL_H
#define ANYPROFILELISTMODEL_H

#include "followslistmodel.h"

class AnyProfileListModel : public FollowsListModel
{
    Q_OBJECT

    Q_PROPERTY(QString targetUri READ targetUri WRITE setTargetUri NOTIFY targetUriChanged)
    Q_PROPERTY(AnyProfileListModelType type READ type WRITE setType NOTIFY typeChanged)

public:
    explicit AnyProfileListModel(QObject *parent = nullptr);

    enum AnyProfileListModelType {
        Like,
        Repost,
    };
    Q_ENUM(AnyProfileListModelType);

    QString targetUri() const;
    void setTargetUri(const QString &newTargetUri);
    AnyProfileListModelType type() const;
    void setType(const AnyProfileListModelType &newType);

public slots:
    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void getNext();

signals:
    void targetUriChanged();
    void typeChanged();

private:
    QString m_targetUri;
    AnyProfileListModelType m_type;
};

#endif // ANYPROFILELISTMODEL_H
