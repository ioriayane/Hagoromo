#ifndef CALENDARTABLEMODEL_H
#define CALENDARTABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>

class CalendarTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(int year READ year WRITE setYear NOTIFY yearChanged FINAL)
    Q_PROPERTY(int month READ month WRITE setMonth NOTIFY monthChanged FINAL)

public:
    explicit CalendarTableModel(QObject *parent = nullptr);

    //
    enum CalendarTableModelRoles {
        ModelData = Qt::UserRole + 1,
        YearRole,
        MonthRole,
        DayRole,
        IsCurrentMonthRole,
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

    int year() const;
    void setYear(int newYear);
    int month() const;
    void setMonth(int newMonth);

signals:
    void yearChanged();
    void monthChanged();

private:
    int m_year;
    int m_month;
};

#endif // CALENDARTABLEMODEL_H
