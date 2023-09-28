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

public slots:
    void process(HttpReply *reply);

signals:
    void finished(bool success);

private:
    class Private;
    Private *d;
    Q_DISABLE_COPY(HttpAccess)
};

#endif // HTTPACCESS_H
