#include "imageeditrenderer.h"

#include "content/imagecompressor.h"

#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
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
    renderLayers(result, state.layers, 1.0, qMax(image.width(), image.height()));
    if (!state.crop.isNull()) {
        QRect crop = state.crop.intersected(result.rect());
        if (!crop.isEmpty() && crop != result.rect()) {
            result = result.copy(crop);
        }
    }
    return result;
}

void ImageEditRenderer::renderLayers(QImage &image, const QList<ImageEditLayer> &layers,
                                     qreal scale, int long_edge)
{
    if (layers.isEmpty() || image.isNull())
        return;
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32
        && image.format() != QImage::Format_ARGB32_Premultiplied) {
        image = image.convertToFormat(image.hasAlphaChannel() ? QImage::Format_ARGB32
                                                              : QImage::Format_RGB32);
    }
    for (const auto &layer : layers) {
        const QRectF rect(layer.rect.x() * scale, layer.rect.y() * scale,
                          layer.rect.width() * scale, layer.rect.height() * scale);
        if (layer.type == ImageEditLayer::Mosaic) {
            const int block = qMax(1, qRound(mosaicBlockSize(layer.strength, long_edge) * scale));
            applyMosaic(image, rect, layer.shape, block);
        }
    }
}

int ImageEditRenderer::mosaicBlockSize(int strength, int long_edge)
{
    strength = qBound(1, strength, 10);
    return qMax(4, qRound(long_edge * strength / 300.0));
}

void ImageEditRenderer::applyMosaic(QImage &image, const QRectF &rect, int shape, int block_size)
{
    const QRect area = rect.toAlignedRect().intersected(image.rect());
    if (area.isEmpty())
        return;
    block_size = qMax(1, block_size);

    // 縮小（平均化）してから最近傍で拡大してブロックにする
    const int w = (area.width() + block_size - 1) / block_size;
    const int h = (area.height() + block_size - 1) / block_size;
    const QImage small =
            image.copy(area).scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    const QImage blocks = small.scaled(w * block_size, h * block_size, Qt::IgnoreAspectRatio,
                                       Qt::FastTransformation);

    QPainter painter(&image);
    // 透過画像は元の画素が透けないように置き換える（境界のアンチエイリアスはしない）
    const bool alpha = image.hasAlphaChannel();
    if (shape == ImageEditLayer::Ellipse) {
        QPainterPath path;
        path.addEllipse(rect);
        painter.setRenderHint(QPainter::Antialiasing, !alpha);
        painter.setClipPath(path);
    }
    if (alpha) {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
    }
    painter.drawImage(area.topLeft(), blocks, QRect(0, 0, area.width(), area.height()));
}

bool ImageEditRenderer::contains(const ImageEditLayer &layer, const QPointF &point)
{
    const QRectF rect(layer.rect);
    if (!rect.contains(point))
        return false;
    if (layer.shape == ImageEditLayer::Ellipse) {
        const qreal rx = rect.width() / 2;
        const qreal ry = rect.height() / 2;
        if (rx <= 0 || ry <= 0)
            return false;
        const qreal dx = (point.x() - rect.center().x()) / rx;
        const qreal dy = (point.y() - rect.center().y()) / ry;
        return (dx * dx + dy * dy) <= 1.0;
    }
    return true;
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
