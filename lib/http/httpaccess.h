#ifndef HTTPACCESS_H
#define HTTPACCESS_H

#include "httpreply.h"

#include <QObject>

class HttpAccess : public QObject
{
    Q_OBJECT
public:
    explicit HttpAccess(QObject *parent = nullptr);
    ~HttpAccess();

    bool get(HttpReply *reply);
    bool post(HttpReply *reply);

public slots:
    void process(HttpReply *reply);

signals:
    void finished(bool success);
};

#endif // HTTPACCESS_H
