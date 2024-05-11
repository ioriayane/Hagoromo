#ifndef LOGDAILYLISTMODEL_H
#define LOGDAILYLISTMODEL_H

#include "logstatisticslistmodel.h"

class LogDailyListModel : public LogStatisticsListModel
{
    Q_OBJECT
public:
    explicit LogDailyListModel(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();
};

#endif // LOGDAILYLISTMODEL_H
