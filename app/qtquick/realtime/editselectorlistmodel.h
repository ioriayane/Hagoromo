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
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
public:
    explicit EditSelectorListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum EditSelectorListModelRoles {
        ModelData = Qt::UserRole + 1,
        TypeRole,
        DisplayTypeRole,
        CanHaveRole,
        IndentRole,
        ListUriRole,
        ListNameRole,
        HasImageRole,
        ImageCountRole,
        HasMovieRole,
        MovieCountRole,
        HasQuoteRole,
    };
    Q_ENUM(EditSelectorListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant item(int row,
                              EditSelectorListModel::EditSelectorListModelRoles role) const;
    Q_INVOKABLE void update(int row, EditSelectorListModel::EditSelectorListModelRoles role,
                            const QVariant &value);
    Q_INVOKABLE void updateList(int row, const QString &name, const QString &uri);

    Q_INVOKABLE void appendChild(int row, const QString &type);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void save(const QString &uuid);

    bool validate() const;
    QString toJson() const;

    QString selectorJson() const;
    void setSelectorJson(const QString &newSelectorJson);
    bool valid() const;
    void setValid(bool newValid);
    int count() const;
    void setCount(int newCount);
    QString name() const;
    void setName(const QString &newName);

signals:
    void selectorJsonChanged();
    void validChanged();
    void countChanged();
    void nameChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void loadSelector();

    RealtimeFeed::AbstractPostSelector *m_selector;
    QString m_selectorJson;
    bool m_valid;
    int m_count;
    QString m_name;
};

#endif // EDITSELECTORLISTMODEL_H
