#include "imageeditrenderer.h"

#include "content/imagecompressor.h"

#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QTransform>
#include <QUuid>
#include <QtMath>

QRectF ImageEditLayer::bounds() const
{
    if (isPointBased()) {
        if (points.isEmpty())
            return QRectF();
        qreal x1 = points.first().x();
        qreal y1 = points.first().y();
        qreal x2 = x1;
        qreal y2 = y1;
        for (const auto &p : points) {
            x1 = qMin(x1, p.x());
            y1 = qMin(y1, p.y());
            x2 = qMax(x2, p.x());
            y2 = qMax(y2, p.y());
        }
        return QRectF(x1, y1, x2 - x1, y2 - y1);
    }
    return QRectF(rect);
}

namespace {
// 点と線分の距離
qreal distanceToSegment(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const QPointF ab = b - a;
    const qreal len2 = ab.x() * ab.x() + ab.y() * ab.y();
    qreal t = 0;
    if (len2 > 0) {
        t = qBound(0.0, QPointF::dotProduct(p - a, ab) / len2, 1.0);
    }
    const QPointF d = p - (a + ab * t);
    return qSqrt(d.x() * d.x() + d.y() * d.y());
}
}

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

QPointF ImageEditRenderer::rotatePoint90(const QPointF &point, const QSize &size, bool clockwise)
{
    if (clockwise) {
        // (x, y) -> (H - y, x)
        return QPointF(size.height() - point.y(), point.x());
    } else {
        // (x, y) -> (y, W - x)
        return QPointF(point.y(), size.width() - point.x());
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
        if (layer.type == ImageEditLayer::Mosaic) {
            const QRectF rect(layer.rect.x() * scale, layer.rect.y() * scale,
                              layer.rect.width() * scale, layer.rect.height() * scale);
            const int block = qMax(1, qRound(mosaicBlockSize(layer.strength, long_edge) * scale));
            applyMosaic(image, rect, layer.shape, block);
        } else {
            drawLayer(image, layer, scale, long_edge);
        }
    }
}

qreal ImageEditRenderer::lineWidthPixels(int line_width, int long_edge)
{
    line_width = qBound(1, line_width, 20);
    return qMax(1.0, long_edge * line_width / 400.0);
}

qreal ImageEditRenderer::arrowHeadLength(int line_width, int long_edge)
{
    return qMax(lineWidthPixels(line_width, long_edge) * 3, long_edge / 80.0);
}

void ImageEditRenderer::drawLayer(QImage &image, const ImageEditLayer &layer, qreal scale,
                                  int long_edge)
{
    if (layer.type == ImageEditLayer::Stamp) {
        if (layer.stampImage.isNull())
            return;
        QPainter painter(&image);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.setOpacity(qBound(10, layer.opacity, 100) / 100.0);
        const QRectF target(layer.rect.x() * scale, layer.rect.y() * scale,
                            layer.rect.width() * scale, layer.rect.height() * scale);
        painter.drawImage(target, layer.stampImage);
        return;
    }
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(layer.color);
    pen.setWidthF(lineWidthPixels(layer.lineWidth, long_edge) * scale);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(layer.fill ? QBrush(layer.color) : QBrush(Qt::NoBrush));

    const QRectF rect(layer.rect.x() * scale, layer.rect.y() * scale, layer.rect.width() * scale,
                      layer.rect.height() * scale);
    if (layer.type == ImageEditLayer::DrawRectangle) {
        painter.drawRect(rect);
    } else if (layer.type == ImageEditLayer::DrawEllipse) {
        painter.drawEllipse(rect);
    } else if (layer.type == ImageEditLayer::Arrow && layer.points.count() == 2) {
        const QPointF start = layer.points.at(0) * scale;
        const QPointF end = layer.points.at(1) * scale;
        const QPointF delta = end - start;
        const qreal length = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
        if (length <= 0) {
            painter.drawPoint(end);
            return;
        }
        const QPointF dir = delta / length;
        const QPointF normal(-dir.y(), dir.x());
        // 三角形は線の長さを超えない
        const qreal head = qMin(arrowHeadLength(layer.lineWidth, long_edge) * scale, length);
        const QPointF base = end - dir * head;
        // 線は三角形の中ほどまで（先端から線がはみ出さないように）
        painter.drawLine(start, end - dir * (head / 2));
        QPolygonF triangle;
        triangle << end << base + normal * (head / 2) << base - normal * (head / 2);
        painter.setPen(Qt::NoPen);
        painter.setBrush(layer.color);
        painter.drawPolygon(triangle);
    } else if (layer.isPointBased() && !layer.points.isEmpty()) {
        if (layer.points.count() == 1) {
            painter.drawPoint(layer.points.first() * scale);
        } else {
            QPainterPath path(layer.points.first() * scale);
            for (int i = 1; i < layer.points.count(); i++) {
                path.lineTo(layer.points.at(i) * scale);
            }
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
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

bool ImageEditRenderer::contains(const ImageEditLayer &layer, const QPointF &point, qreal tolerance,
                                 int long_edge)
{
    if (layer.isDrawing()) {
        qreal margin = lineWidthPixels(layer.lineWidth, long_edge) / 2;
        if (layer.type == ImageEditLayer::Arrow) {
            // 三角形の部分もつかめるように
            margin = qMax(margin, arrowHeadLength(layer.lineWidth, long_edge) / 2);
        }
        margin += tolerance;
        if (!layer.bounds().adjusted(-margin, -margin, margin, margin).contains(point))
            return false;
        if (layer.isPointBased()) {
            if (layer.points.count() == 1) {
                return distanceToSegment(point, layer.points.first(), layer.points.first())
                        <= margin;
            }
            for (int i = 1; i < layer.points.count(); i++) {
                if (distanceToSegment(point, layer.points.at(i - 1), layer.points.at(i)) <= margin)
                    return true;
            }
            return false;
        }
        const QRectF rect(layer.rect);
        if (layer.type == ImageEditLayer::DrawRectangle) {
            if (layer.fill)
                return true; // 外側の判定は済み
            // 枠線の近く
            return !rect.adjusted(margin, margin, -margin, -margin).contains(point);
        }
        // 楕円: 中心からの正規化した距離で判定
        const qreal rx = rect.width() / 2;
        const qreal ry = rect.height() / 2;
        if (rx <= 0 || ry <= 0)
            return true;
        const qreal dx = (point.x() - rect.center().x()) / rx;
        const qreal dy = (point.y() - rect.center().y()) / ry;
        const qreal d = qSqrt(dx * dx + dy * dy);
        if (layer.fill)
            return d <= 1.0 + margin / qMin(rx, ry);
        return qAbs(d - 1.0) * qMin(rx, ry) <= margin;
    }

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
