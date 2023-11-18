#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>

namespace Common {

inline QString appDataFolder()
{
    return QString("%1/%2/%3%4")
            .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
            .arg(QCoreApplication::organizationName())
            .arg(QCoreApplication::applicationName())
            .arg(
#if defined(HAGOROMO_UNIT_TEST)
                    QStringLiteral("_unittest")
#elif defined(QT_DEBUG)
                    QStringLiteral("_debug")
#else
                    QString()
#endif
            );
}

inline QString appTempFolder(const QString &sub = QString())
{
    QString folder =
            QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation),
                                 QCoreApplication::applicationName());
    if (!sub.isEmpty()) {
        folder.append("/" + sub);
    }
    QDir dir(folder);
    dir.mkpath(folder);
    return folder;
}

inline QString mimeTypeToExtension(const QString &mime_type)
{
    QStringList items = mime_type.split("/");
    if (items.length() == 2) {
        return items.last();
    }
    return mime_type;
}

inline void saveJsonDocument(const QJsonDocument &doc, const QString &file_name)
{
    QString folder = Common::appDataFolder();
    QDir dir(folder);
    dir.mkpath(folder);
    QFile file(QString("%1/%2").arg(folder, file_name));
    if (file.open(QFile::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

inline QJsonDocument loadJsonDocument(const QString &file_name)
{
    QByteArray byte_array;
    QString folder = Common::appDataFolder();
    QFile file(QString("%1/%2").arg(folder, file_name));
    if (file.open(QFile::ReadOnly)) {
        byte_array = file.readAll();
        file.close();
    }
    return QJsonDocument::fromJson(byte_array);
}

}
#endif // COMMON_H
