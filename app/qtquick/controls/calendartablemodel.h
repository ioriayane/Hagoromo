#ifndef CALENDARTABLEMODEL_H
#define CALENDARTABLEMODEL_H

#include "qdatetime.h"
#include <QAbstractTableModel>
#include <QObject>

class CalendarTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(int year READ year WRITE setYear NOTIFY yearChanged FINAL)
    Q_PROPERTY(int month READ month WRITE setMonth NOTIFY monthChanged FINAL)

    Q_PROPERTY(QString since READ since WRITE setSince NOTIFY sinceChanged FINAL)
    Q_PROPERTY(QString until READ until WRITE setUntil NOTIFY untilChanged FINAL)
    Q_PROPERTY(
            bool enableSince READ enableSince WRITE setEnableSince NOTIFY enableSinceChanged FINAL)
    Q_PROPERTY(
            bool enableUntil READ enableUntil WRITE setEnableUntil NOTIFY enableUntilChanged FINAL)
public:
    explicit CalendarTableModel(QObject *parent = nullptr);

    //
    enum CalendarTableModelRoles {
        ModelData = Qt::UserRole + 1,
        YearRole,
        MonthRole,
        DayRole,
        IsCurrentMonthRole,
        EnabledRole,
    };
    Q_ENUM(CalendarTableModelRoles)

    int rowCount(const QModelIndex &row = QModelIndex()) const;
    int columnCount(const QModelIndex &column = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE QVariant item(int row, int column,
                              CalendarTableModel::CalendarTableModelRoles role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void addYears(int years);
    Q_INVOKABLE void addMonths(int months);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void setSinceDate(int year, int month, int day);
    Q_INVOKABLE void setUntilDate(int year, int month, int day);

    int year() const;
    void setYear(int newYear);
    int month() const;
    void setMonth(int newMonth);
    QString since() const;
    void setSince(const QString &newSince);
    QString until() const;
    bool enableSince() const;
    void setEnableSince(bool newEnableSince);
    bool enableUntil() const;
    void setEnableUntil(bool newEnableUntil);

    void setUntil(const QString &newUntil);

signals:
    void yearChanged();
    void monthChanged();
    void sinceChanged();
    void untilChanged();
    void enableSinceChanged();
    void enableUntilChanged();

private:
    int m_year;
    int m_month;
    QDate m_since;
    QDate m_until;
    bool m_enableSince;
    bool m_enableUntil;
};

#endif // CALENDARTABLEMODEL_H
