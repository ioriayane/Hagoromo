#ifndef CONTENTFILTERSETTINGLISTMODEL_H
#define CONTENTFILTERSETTINGLISTMODEL_H

#include "tools/configurablelabels.h"

#include <QAbstractListModel>

class ContentFilterSettingListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool enableAdultContent READ enableAdultContent WRITE setEnableAdultContent NOTIFY
                       enableAdultContentChanged)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(QString handle READ handle WRITE setHandle NOTIFY handleChanged)
    Q_PROPERTY(QString accessJwt READ accessJwt WRITE setAccessJwt NOTIFY accessJwtChanged)
    // モデルとして提供されるラベラーのDIDを設定する
    Q_PROPERTY(
            QString labelerDid READ labelerDid WRITE setLabelerDid NOTIFY labelerDidChanged FINAL)
    // laberDidに設定できるDIDの一覧
    Q_PROPERTY(QStringList selectableLabelerDids READ selectableLabelerDids WRITE
                       setSelectableLabelerDids NOTIFY selectableLabelerDidsChanged FINAL)
    Q_PROPERTY(bool labelerHasAdultOnly READ labelerHasAdultOnly WRITE setLabelerHasAdultOnly NOTIFY
                       labelerHasAdultOnlyChanged FINAL)
    Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged FINAL)

public:
    explicit ContentFilterSettingListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ContentFilterSettingListModelRoles {
        ModelData = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        StatusRole,
        LevelRole,
        IsAdultImageryRole,
        ConfigurableRole,
    };
    Q_ENUM(ContentFilterSettingListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant
    item(int row, ContentFilterSettingListModel::ContentFilterSettingListModelRoles role) const;
    Q_INVOKABLE void update(int row,
                            ContentFilterSettingListModel::ContentFilterSettingListModelRoles role,
                            const QVariant &value);

    Q_INVOKABLE void load();
    Q_INVOKABLE void save();
    Q_INVOKABLE void clear();

    Q_INVOKABLE QString selectableLabelerName(const QString &did) const;
    Q_INVOKABLE QString selectableLabelerDescription(const QString &did) const;

    bool enableAdultContent() const;
    void setEnableAdultContent(bool newEnableAdultContent);
    bool running() const;
    QString service() const;
    void setService(const QString &newService);
    QString handle() const;
    void setHandle(const QString &newHandle);
    QString accessJwt() const;
    void setAccessJwt(const QString &newAccessJwt);

    QString labelerDid() const;
    void setLabelerDid(const QString &newLabelerDid);
    QStringList selectableLabelerDids() const;
    void setSelectableLabelerDids(const QStringList &newSelectableLabelerDids);

    bool labelerHasAdultOnly() const;
    void setLabelerHasAdultOnly(bool newLabelerHasAdultOnly);

    bool modified() const;
    void setModified(bool newModified);

public slots:
    void setRunning(bool newRunning);

signals:
    void finished();
    void enableAdultContentChanged();
    void runningChanged();
    void serviceChanged();
    void handleChanged();
    void accessJwtChanged();

    void labelerDidChanged();

    void selectableLabelerDidsChanged();

    void labelerHasAdultOnlyChanged();

    void modifiedChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

    ConfigurableLabels m_contentFilterLabels;

private:
    bool m_saving;
    bool m_enableAdultContent;
    bool m_running;
    QString m_service;
    QString m_handle;
    QString m_accessJwt;
    QString m_labelerDid;
    QStringList m_selectableLabelerDids;
    bool m_labelerHasAdultOnly;
    bool m_modified;
};

#endif // CONTENTFILTERSETTINGLISTMODEL_H
