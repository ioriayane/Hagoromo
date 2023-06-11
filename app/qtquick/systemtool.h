#ifndef SYSTEMTOOL_H
#define SYSTEMTOOL_H

#include <QObject>

class SystemTool : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString applicationVersion READ applicationVersion CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)

public:
    explicit SystemTool(QObject *parent = nullptr);

    QString applicationVersion() const;
    QString qtVersion() const;
};

#endif // SYSTEMTOOL_H
