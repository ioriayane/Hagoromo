#ifndef LOGACCESS_H
#define LOGACCESS_H

#include <QObject>

struct TotalItem
{
    TotalItem() { }
    TotalItem(const TotalItem &other)
    {
        this->group = other.group;
        this->name = other.name;
        this->count = other.count;
    }
    TotalItem(const QString &group, const QString &name)
    {
        this->group = group;
        this->name = name;
    }
    QString group;
    QString name;
    int count = 0;
};
Q_DECLARE_METATYPE(TotalItem)

struct RecordPostItem
{
    QString uri;
    QString json;
};
Q_DECLARE_METATYPE(RecordPostItem)

class LogAccess : public QObject
{
    Q_OBJECT
public:
    explicit LogAccess(QObject *parent = nullptr);
    ~LogAccess();

    void removeDbFile(const QString &did);

public slots:
    void updateDb(const QString &did, const QByteArray &data);
    void dailyTotals(const QString &did);
    void monthlyTotals(const QString &did);
    void statistics(const QString &did);
    // kind, 0:daily, 1:montyly, 2:words
    void selectRecords(const QString &did, const int kind, const QString &condition,
                       const QString &cursor, const int limit);
    void updateRecords(const QString &did, const QList<RecordPostItem> &record_post_items);
    void setVersion(const QString &did, const int version);
    int getVersion(const QString &did);

signals:
    void progressMessage(const QString &message);
    void finishedUpdateDb(bool success);
    void finishedTotals(const QList<TotalItem> &list, const int max);
    void finishedSelection(const QString &records, const QStringList &view_posts);
    void finishedUpdateRecords();

private:
    class Private;
    Private *d;
    Q_DISABLE_COPY(LogAccess)
};

#endif // LOGACCESS_H
