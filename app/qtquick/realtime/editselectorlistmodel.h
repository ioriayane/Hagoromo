#ifndef EDITSELECTORLISTMODEL_H
#define EDITSELECTORLISTMODEL_H

#include <QAbstractListModel>

#include "realtime/abstractpostselector.h"

class EditSelectorListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString selectorJson READ selectorJson WRITE setSelectorJson NOTIFY
                       selectorJsonChanged FINAL)
    Q_PROPERTY(bool valid READ valid WRITE setValid NOTIFY validChanged FINAL)
public:
    explicit EditSelectorListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum EditSelectorListModelRoles {
        ModelData = Qt::UserRole + 1,
        TypeRole,
        DisplayTypeRole,
        CanContainRole,
        IndentRole,
    };
    Q_ENUM(EditSelectorListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant item(int row,
                              EditSelectorListModel::EditSelectorListModelRoles role) const;
    Q_INVOKABLE void appendChild(int row, const QString &type);
    Q_INVOKABLE void clear();

    bool validate() const;
    QString toJson() const;

    QString selectorJson() const;
    void setSelectorJson(const QString &newSelectorJson);
    bool valid() const;
    void setValid(bool newValid);

signals:
    void selectorJsonChanged();
    void validChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void loadSelector();

    RealtimeFeed::AbstractPostSelector *m_selector;
    QString m_selectorJson;
    bool m_valid;
};

#endif // EDITSELECTORLISTMODEL_H
