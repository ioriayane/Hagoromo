#ifndef LANGUAGELISTMODEL_H
#define LANGUAGELISTMODEL_H

#include <QAbstractListModel>

struct LanguageItem
{
    LanguageItem(const QString &name, const QString &iso638_1)
    {
        this->name = name;
        this->iso639_1 = iso638_1;
    }
    QString name;
    QString iso639_1;
    bool checked = false;
    bool enabled = true;
};

class LanguageListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit LanguageListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum LanguageListModelRoles {
        ModelData = Qt::UserRole + 1,
        NameRole,
        CheckedRole,
        EnabledRole
    };
    Q_ENUM(LanguageListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, LanguageListModel::LanguageListModelRoles role) const;
    Q_INVOKABLE void update(int row, LanguageListModel::LanguageListModelRoles role,
                            const QVariant &value);
    Q_INVOKABLE void setSelectedLanguages(const QStringList &langs);
    Q_INVOKABLE QStringList selectedLanguages() const;
    Q_INVOKABLE QStringList convertLanguageNames(const QStringList &langs) const;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void updateEnable();

    QList<LanguageItem> m_languageList;
    QHash<QString, int> m_iso639toIndexHash;
};

#endif // LANGUAGELISTMODEL_H
