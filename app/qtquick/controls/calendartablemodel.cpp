#include "calendartablemodel.h"

#include <QCalendar>
#include <QDate>
#include <QDebug>

CalendarTableModel::CalendarTableModel(QObject *parent)
    : QAbstractTableModel { parent },
      m_year(QDate::currentDate().year()),
      m_month(QDate::currentDate().month())
{
}

int CalendarTableModel::rowCount(const QModelIndex &row) const
{
    return 6;
}

int CalendarTableModel::columnCount(const QModelIndex &column) const
{
    return 7;
}

QVariant CalendarTableModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), index.column(), static_cast<CalendarTableModelRoles>(role));
}

QVariant CalendarTableModel::item(int row, int column, CalendarTableModelRoles role) const
{
    QDate start_date = QDate(year(), month(), 1);
    QDate index_date;
    // qDebug().noquote() << "r=" << index.row() << "c=" << index.column() << "day of weed"
    //                    << start_date.dayOfWeek();

    int total = start_date.daysInMonth()
            + (start_date.dayOfWeek() == 7 ? 0 : (start_date.dayOfWeek() + 1));
    int enable_row_count =
            ((total % 7 == 0) ? static_cast<int>(total / 7) : static_cast<int>(total / 7) + 1);
    // qDebug().noquote() << "rowCount" << int(total / 7) << total << start_date.daysInMonth()
    //                    << QCalendar().daysInMonth(month(), year()) << enable_row_count;

    if (row >= enable_row_count)
        return QString();

    if (start_date.dayOfWeek() == 7) {
        index_date = start_date.addDays(row * 7 + column);
    } else {
        index_date = start_date.addDays(row * 7 + column - start_date.dayOfWeek());
    }
    if (index_date.year() < 0)
        return QString(); // 紀元前には対応しない

    switch (role) {
    case YearRole:
        return QString::number(index_date.year());
    case MonthRole:
        return QString::number(index_date.month());
    case DayRole:
        return QString::number(index_date.day());
    case IsCurrentMonthRole:
        return index_date.month() == month();
    default:
        break;
    }

    return QString();
}

QHash<int, QByteArray> CalendarTableModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[YearRole] = "year";
    roles[MonthRole] = "month";
    roles[DayRole] = "day";
    roles[IsCurrentMonthRole] = "isCurrentMonth";

    return roles;
}

void CalendarTableModel::addYears(int years)
{
    QDate d = QDate(year(), month(), 1).addYears(years);
    if (d.year() < 0) {
        setYear(1);
        setMonth(d.month());
    } else {
        setYear(d.year());
        setMonth(d.month());
    }
}

void CalendarTableModel::addMonths(int months)
{
    QDate d = QDate(year(), month(), 1).addMonths(months);
    if (d.year() < 0) {
        setYear(1);
        setMonth(1);
    } else {
        setYear(d.year());
        setMonth(d.month());
    }
}

int CalendarTableModel::year() const
{
    return m_year;
}

void CalendarTableModel::setYear(int newYear)
{
    if (m_year == newYear)
        return;
    m_year = newYear;
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
    emit yearChanged();
}

int CalendarTableModel::month() const
{
    return m_month;
}

void CalendarTableModel::setMonth(int newMonth)
{
    if (m_month == newMonth)
        return;
    m_month = newMonth;
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
    emit monthChanged();
}
