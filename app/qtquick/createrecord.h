#ifndef CREATERECORD_H
#define CREATERECORD_H

#include "createsession.h"
#include <QObject>

class CreateRecord : public CreateSession
{
    Q_OBJECT

public:
    explicit CreateRecord(QObject *parent = nullptr);

    Q_INVOKABLE void post(const QString &text) const;
signals:
};

#endif // CREATERECORD_H
