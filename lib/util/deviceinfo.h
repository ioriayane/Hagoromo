#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>

namespace DeviceInfo {

/**
 * @brief Get the MAC address of the first active non-loopback network interface
 * @return MAC address string (e.g., "00:11:22:33:44:55"), or empty string if not found
 */
QString getMacAddress();

/**
 * @brief Get a unique device identifier based on hashed MAC address
 * @return SHA256 hash of MAC address as hex string, or empty string if not found
 */
QString getDeviceId();

/**
 * @brief Get the machine host name (computer name)
 * @return Host name string, or empty string if not available
 */
QString getDeviceName();

} // namespace DeviceInfo

#endif // DEVICEINFO_H
