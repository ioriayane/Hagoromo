#include "deviceinfo.h"
#include <QCryptographicHash>
#include <QNetworkInterface>
#include <QSysInfo>

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
    QString macAddress = getMacAddress();
    if (macAddress.isEmpty()) {
        return QString();
    }

    // Hash the MAC address using SHA256
    QByteArray hash = QCryptographicHash::hash(macAddress.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromLatin1(hash.toHex());
}

QString getDeviceName()
{
    return QSysInfo::machineHostName();
}

} // namespace DeviceInfo
