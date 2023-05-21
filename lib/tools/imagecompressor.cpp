#include "imagecompressor.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QImage>
#include <QUuid>
#include <QCoreApplication>
#include <QDir>

ImageCompressor::ImageCompressor(QObject *parent) : QObject { parent } { }

void ImageCompressor::compress(const QString &path)
{
    QFileInfo info(path);
    if (info.size() < 1000000) {
        emit compressed(path);
        return;
    }

    QString folder =
            QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation),
                                 QCoreApplication::applicationName());
    QDir dir(folder);
    dir.mkpath(folder);

    QImage src(path);
    QString new_path = QString("%1/%2.jpg").arg(folder, QUuid::createUuid().toString(QUuid::Id128));

    QFileInfo new_info(path);
    for (int quality = 90; quality >= 70; quality -= 5) {
        // qDebug() << new_info.size() << "/" << info.size() << "," << quality;
        if (new_info.size() < 1000000) {
            break;
        }
        src.save(new_path, nullptr, quality);
        new_info.setFile(new_path);
    }
    for (qreal ratio = 0.9; ratio >= 0.1; ratio -= 0.1) {
        // qDebug() << new_info.size() << "/" << info.size() << "," << src.width() << "x"
        //         << src.height();
        if (new_info.size() < 1000000) {
            break;
        }
        src.scaled(src.width() * ratio, src.height() * ratio, Qt::KeepAspectRatio)
                .save(new_path, nullptr, 70);
        new_info.setFile(new_path);
    }

    emit compressed(new_path);
}
