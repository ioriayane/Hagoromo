#include "imageeditrenderer.h"

#include "content/imagecompressor.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTransform>
#include <QUuid>

QImage ImageEditRenderer::load(const QString &path)
{
    return ImageCompressor::loadImage(path);
}

int ImageEditRenderer::normalizeRotation(int rotation)
{
    rotation = rotation % 360;
    if (rotation < 0)
        rotation += 360;
    // 90度単位に丸める
    return ((rotation + 45) / 90 % 4) * 90;
}

QSize ImageEditRenderer::rotatedSize(const QSize &size, int rotation)
{
    rotation = normalizeRotation(rotation);
    if (rotation == 90 || rotation == 270) {
        return size.transposed();
    }
    return size;
}

QImage ImageEditRenderer::rotate(const QImage &image, int rotation)
{
    rotation = normalizeRotation(rotation);
    if (rotation == 0 || image.isNull()) {
        return image;
    }
    QTransform transform;
    transform.rotate(rotation);
    return image.transformed(transform);
}

QRect ImageEditRenderer::rotateRect90(const QRect &rect, const QSize &size, bool clockwise)
{
    if (rect.isNull())
        return rect;
    if (clockwise) {
        // (x, y) -> (H - y, x)
        return QRect(size.height() - rect.y() - rect.height(), rect.x(), rect.height(),
                     rect.width());
    } else {
        // (x, y) -> (y, W - x)
        return QRect(rect.y(), size.width() - rect.x() - rect.width(), rect.height(), rect.width());
    }
}

QImage ImageEditRenderer::render(const QImage &image, const ImageEditState &state)
{
    if (image.isNull())
        return image;

    QImage result = rotate(image, state.rotation);
    if (!state.crop.isNull()) {
        QRect crop = state.crop.intersected(result.rect());
        if (!crop.isEmpty() && crop != result.rect()) {
            result = result.copy(crop);
        }
    }
    return result;
}

QString ImageEditRenderer::editedFolder()
{
    return QString("%1/edited")
            .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
}

bool ImageEditRenderer::isEditedFile(const QString &path)
{
    if (path.isEmpty())
        return false;
    QString folder = QDir::cleanPath(QFileInfo(editedFolder()).absoluteFilePath());
    QString dir = QDir::cleanPath(QFileInfo(path).absolutePath());
    return (folder == dir);
}

QString ImageEditRenderer::save(const QImage &image, const QString &folder)
{
    if (image.isNull())
        return QString();

    QString dir_path = folder.isEmpty() ? editedFolder() : folder;
    QDir dir(dir_path);
    if (!dir.mkpath(dir_path))
        return QString();

    const bool alpha = image.hasAlphaChannel();
    QString path = QString("%1/%2.%3")
                           .arg(dir_path, QUuid::createUuid().toString(QUuid::Id128),
                                alpha ? QStringLiteral("png") : QStringLiteral("jpg"));
    bool ret = alpha ? image.save(path, "PNG") : image.save(path, "JPEG", 95);
    if (!ret) {
        QFile::remove(path);
        return QString();
    }
    return path;
}
