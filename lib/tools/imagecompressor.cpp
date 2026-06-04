#include "imagecompressor.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QImage>
#include <QUuid>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

#define IMAGE_SIZE_LIMIT 2000000

ImageCompressor::ImageCompressor(QObject *parent) : QObject { parent } { }

void ImageCompressor::compress(const QString &path, int image_size_limit)
{
    if (image_size_limit <= 0) {
        image_size_limit = IMAGE_SIZE_LIMIT;
    }
    QString folder =
            QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation),
                                 QCoreApplication::applicationName());
    QDir dir(folder);
    dir.mkpath(folder);

    QImage src(path);
    QString new_path = QString("%1/%2.jpg").arg(folder, QUuid::createUuid().toString(QUuid::Id128));

    QFileInfo new_info(path);
    if (new_info.suffix().toLower() == "gif" && new_info.size() < image_size_limit) {
        // gifの場合はそのまま
        qDebug() << "Detect gif file. " << path;
        new_path = path;
    } else {
        for (int quality = 90; quality >= 70; quality -= 5) {
            src.save(new_path, nullptr, quality);
            new_info.setFile(new_path);
            // qDebug() << new_info.size() << "/" << info.size() << "," << quality;
            if (new_info.size() < image_size_limit) {
                break;
            }
        }
        for (qreal ratio = 0.9; ratio >= 0.1; ratio -= 0.1) {
            // qDebug() << new_info.size() << "/" << info.size() << "," << src.width() << "x"
            //         << src.height();
            if (new_info.size() < image_size_limit) {
                break;
            }
            src.scaled(src.width() * ratio, src.height() * ratio, Qt::KeepAspectRatio)
                    .save(new_path, nullptr, 70);
            new_info.setFile(new_path);
        }
    }
    QImage new_img(new_path);

    emit compressed(new_path, new_img.size());
}
