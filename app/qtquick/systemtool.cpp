#include "systemtool.h"

#include <QClipboard>
#include <QGuiApplication>

SystemTool::SystemTool(QObject *parent) : QObject { parent } { }

void SystemTool::copyToClipboard(const QString &text) const
{
    QGuiApplication::clipboard()->setText(text);
}

QString SystemTool::applicationVersion() const
{
    return QCoreApplication::applicationVersion();
}

QString SystemTool::qtVersion() const
{
    return QT_VERSION_STR;
}
