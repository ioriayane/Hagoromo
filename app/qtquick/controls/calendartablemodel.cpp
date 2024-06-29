#include "calendartablemodel.h"

#include <QCalendar>
#include <QDate>
#include <QDebug>

CalendarTableModel::CalendarTableModel(QObject *parent)
    : QAbstractTableModel { parent },
      m_year(QDate::currentDate().year()),
      m_month(QDate::currentDate().month()),
      m_since(QDate::currentDate()),
      m_until(QDate::currentDate()),
      m_enableSince(false),
      m_enableUntil(false)
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
    QDate until_date;

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
    case EnabledRole: {
        qDebug().noquote() << "enableSince" << enableSince() << m_since;
        qDebug().noquote() << "enableUntil" << enableUntil() << m_until;
        if (enableSince() && m_since.isValid()) {
            if (index_date < m_since) {
                return false;
            }
        }
        if (enableUntil() && m_until.isValid()) {
            if (m_until < index_date) {
                return false;
            }
        }
        return true;
    }
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
    roles[EnabledRole] = "outOfRange";

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

void CalendarTableModel::clear()
{
    setYear(QDate::currentDate().year());
    setMonth(QDate::currentDate().month());
    // m_since = QDate::currentDate();
    // m_until = QDate::currentDate();
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

QString CalendarTableModel::since() const
{
    return m_since.toString("yyyy/MM/dd");
}

void CalendarTableModel::setSince(const QString &newSince)
{
    Q_UNUSED(newSince)
}

void CalendarTableModel::setSinceDate(int year, int month, int day)
{
    QDate new_date(year, month, day);
    if (m_since == new_date)
        return;
    m_since = new_date;
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
    emit sinceChanged();
}

QString CalendarTableModel::until() const
{
    return m_until.toString("yyyy/MM/dd");
}

void CalendarTableModel::setUntil(const QString &newUntil)
{
    Q_UNUSED(newUntil)
}

void CalendarTableModel::setUntilDate(int year, int month, int day)
{
    QDate new_date(year, month, day);
    if (m_until == new_date)
        return;
    m_until = new_date;
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
    emit untilChanged();
}

bool CalendarTableModel::enableSince() const
{
    return m_enableSince;
}

void CalendarTableModel::setEnableSince(bool newEnableSince)
{
    if (m_enableSince == newEnableSince)
        return;
    m_enableSince = newEnableSince;
    emit enableSinceChanged();
}

bool CalendarTableModel::enableUntil() const
{
    return m_enableUntil;
}

void CalendarTableModel::setEnableUntil(bool newEnableUntil)
{
    if (m_enableUntil == newEnableUntil)
        return;
    m_enableUntil = newEnableUntil;
    emit enableUntilChanged();
}
