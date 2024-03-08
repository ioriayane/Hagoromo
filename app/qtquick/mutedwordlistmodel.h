#ifndef MUTEDWORDLISTMODEL_H
#define MUTEDWORDLISTMODEL_H

#include "contentfiltersettinglistmodel.h"

class MutedWordListModel : public ContentFilterSettingListModel
{
    Q_OBJECT

public:
    explicit MutedWordListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum MutedWordListModelRoles {
        ModelData = Qt::UserRole + 1,
        ValueRole,
        TargetsRole,
    };
    Q_ENUM(MutedWordListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant item(int row, MutedWordListModel::MutedWordListModelRoles role) const;

    bool enableAdultContent() const = delete;
    void setEnableAdultContent(bool newEnableAdultContent) = delete;
public slots:

signals:

protected:
    QHash<int, QByteArray> roleNames() const;

private:
};

#endif // MUTEDWORDLISTMODEL_H
