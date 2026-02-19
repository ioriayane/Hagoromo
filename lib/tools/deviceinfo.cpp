#include "deviceinfo.h"
#include <QCryptographicHash>
#include <QNetworkInterface>
#include <QSettings>
#include <QSysInfo>
#include <QUuid>

namespace DeviceInfo {

QString getMacAddress()
{
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces) {
        if (!(interface.flags() & QNetworkInterface::IsLoopBack)
            && (interface.flags() & QNetworkInterface::IsUp)) {
            QString macAddress = interface.hardwareAddress();
            if (!macAddress.isEmpty()) {
                return macAddress;
            }
        }
    }
    return QString();
}

QString getDeviceId()
{
    QSettings settings;

    // Try to retrieve existing device ID
    QString deviceId = settings.value("deviceId").toString();

    if (!deviceId.isEmpty()) {
        return deviceId;
    }

    // Generate new UUIDv4 if not found
    deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // Save to settings for future use
    settings.setValue("deviceId", deviceId);
    settings.sync();

    return deviceId;
}

QString getDeviceName()
{
    return QSysInfo::machineHostName();
}

} // namespace DeviceInfo
