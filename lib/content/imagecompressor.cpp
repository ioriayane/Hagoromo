#include "imagecompressor.h"

#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QImageReader>
#include <QUuid>
#include <QColorSpace>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

#define IMAGE_SIZE_LIMIT 2000000

ImageCompressor::ImageCompressor(QObject *parent) : QObject { parent } { }

QImage ImageCompressor::loadImage(const QString &path)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    return stripMetadata(reader.read());
}

QImage ImageCompressor::stripMetadata(const QImage &image)
{
    if (image.isNull())
        return image;

    QImage src = image;
    if (src.colorCount() > 0) {
        // カラーテーブルを持つ形式は扱いやすい形式にそろえる
        src = src.convertToFormat(src.hasAlphaChannel() ? QImage::Format_ARGB32
                                                        : QImage::Format_RGB32);
    }
    // バッファから作り直したQImageはテキストなどのメタデータを引き継がない
    QImage result =
            QImage(src.constBits(), src.width(), src.height(), src.bytesPerLine(), src.format())
                    .copy();
    result.setColorSpace(src.colorSpace());
    result.setDotsPerMeterX(src.dotsPerMeterX());
    result.setDotsPerMeterY(src.dotsPerMeterY());
    return result;
}

void ImageCompressor::compress(const QString &path, int image_size_limit, bool keep_format)
{
    if (image_size_limit <= 0) {
        image_size_limit = IMAGE_SIZE_LIMIT;
    }
    QString folder =
            QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation),
                                 QCoreApplication::applicationName());
    QDir dir(folder);
    dir.mkpath(folder);

    QString new_path;
    QFileInfo new_info(path);
    if (new_info.suffix().toLower() == "gif" && new_info.size() < image_size_limit) {
        // gifの場合はアニメーションを維持するためそのまま
        qDebug() << "Detect gif file. " << path;
        new_path = path;
    } else {
        // 位置情報などのメタデータを送信しないように必ず読み込み直して保存する
        QImage src = loadImage(path);
        bool done = false;
        if (keep_format && QImageReader::imageFormat(path) != "jpeg") {
            // 形式を維持する指定（透過PNGなど）
            new_path = QString("%1/%2.png").arg(folder, QUuid::createUuid().toString(QUuid::Id128));
            src.save(new_path, "PNG");
            new_info.setFile(new_path);
            done = (new_info.size() < image_size_limit);
            if (!done) {
                QFile::remove(new_path);
            }
        }
        if (!done) {
            new_path = QString("%1/%2.jpg").arg(folder, QUuid::createUuid().toString(QUuid::Id128));
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
    }
    QImageReader new_img(new_path);

    emit compressed(new_path, new_img.size());
}
