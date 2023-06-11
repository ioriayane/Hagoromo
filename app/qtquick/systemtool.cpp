#include "systemtool.h"

#include <QCoreApplication>

SystemTool::SystemTool(QObject *parent) : QObject { parent } { }

QString SystemTool::applicationVersion() const
{
    return QCoreApplication::applicationVersion();
}

QString SystemTool::qtVersion() const
{
    return QT_VERSION_STR;
}
