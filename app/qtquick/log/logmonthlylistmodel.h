#ifndef LOGMONTHLYLISTMODEL_H
#define LOGMONTHLYLISTMODEL_H

#include "logstatisticslistmodel.h"

class LogMonthlyListModel : public LogStatisticsListModel
{
    Q_OBJECT
public:
    explicit LogMonthlyListModel(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();
};

#endif // LOGMONTHLYLISTMODEL_H
