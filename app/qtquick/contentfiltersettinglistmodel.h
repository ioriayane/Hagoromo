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

public:
    explicit ContentFilterSettingListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ContentFilterSettingListModelRoles {
        ModelData = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        StatusRole,
        IsAdultImageryRole,
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

    bool enableAdultContent() const;
    void setEnableAdultContent(bool newEnableAdultContent);
    bool running() const;
    QString service() const;
    void setService(const QString &newService);
    QString handle() const;
    void setHandle(const QString &newHandle);
    QString accessJwt() const;
    void setAccessJwt(const QString &newAccessJwt);

public slots:
    void setRunning(bool newRunning);

signals:
    void finished();
    void enableAdultContentChanged();
    void runningChanged();
    void serviceChanged();
    void handleChanged();
    void accessJwtChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    ConfigurableLabels m_contentFilterLabels;
    bool m_enableAdultContent;
    bool m_running;
    QString m_service;
    QString m_handle;
    QString m_accessJwt;
};

#endif // CONTENTFILTERSETTINGLISTMODEL_H
