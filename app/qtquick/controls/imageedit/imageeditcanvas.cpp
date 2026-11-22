#include "imageeditcanvas.h"
#include "imageeditmodel.h"

#include <QPainter>

ImageEditCanvas::ImageEditCanvas(QQuickItem *parent)
    : QQuickPaintedItem(parent), m_showWholeImage(false), m_margin(0), m_paintedScale(1.0)
{
}

void ImageEditCanvas::paint(QPainter *painter)
{
    if (!m_editModel)
        return;
    const QImage preview = m_editModel->previewImage();
    const QRectF region = displayRegion();
    if (preview.isNull() || region.isEmpty() || m_paintedRect.isEmpty())
        return;

    const qreal s = m_editModel->previewScale();
    const QRectF source(region.x() * s, region.y() * s, region.width() * s, region.height() * s);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(m_paintedRect, preview, source);
}

QPointF ImageEditCanvas::mapToImage(qreal x, qreal y) const
{
    if (m_paintedScale <= 0)
        return QPointF();
    const QRectF region = displayRegion();
    return QPointF(region.x() + (x - m_paintedRect.x()) / m_paintedScale,
                   region.y() + (y - m_paintedRect.y()) / m_paintedScale);
}

QPointF ImageEditCanvas::mapFromImage(qreal x, qreal y) const
{
    const QRectF region = displayRegion();
    return QPointF(m_paintedRect.x() + (x - region.x()) * m_paintedScale,
                   m_paintedRect.y() + (y - region.y()) * m_paintedScale);
}

ImageEditModel *ImageEditCanvas::editModel() const
{
    return m_editModel;
}

void ImageEditCanvas::setEditModel(ImageEditModel *newEditModel)
{
    if (m_editModel == newEditModel)
        return;
    if (m_editModel) {
        disconnect(m_editModel, nullptr, this, nullptr);
    }
    m_editModel = newEditModel;
    if (m_editModel) {
        connect(m_editModel, &ImageEditModel::previewChanged, this, &ImageEditCanvas::refresh);
        connect(m_editModel, &ImageEditModel::stateChanged, this, &ImageEditCanvas::refresh);
    }
    refresh();
    emit editModelChanged();
}

bool ImageEditCanvas::showWholeImage() const
{
    return m_showWholeImage;
}

void ImageEditCanvas::setShowWholeImage(bool newShowWholeImage)
{
    if (m_showWholeImage == newShowWholeImage)
        return;
    m_showWholeImage = newShowWholeImage;
    refresh();
    emit showWholeImageChanged();
}

qreal ImageEditCanvas::margin() const
{
    return m_margin;
}

void ImageEditCanvas::setMargin(qreal newMargin)
{
    if (qFuzzyCompare(m_margin, newMargin))
        return;
    m_margin = newMargin;
    refresh();
    emit marginChanged();
}

qreal ImageEditCanvas::paintedX() const
{
    return m_paintedRect.x();
}

qreal ImageEditCanvas::paintedY() const
{
    return m_paintedRect.y();
}

qreal ImageEditCanvas::paintedWidth() const
{
    return m_paintedRect.width();
}

qreal ImageEditCanvas::paintedHeight() const
{
    return m_paintedRect.height();
}

qreal ImageEditCanvas::paintedScale() const
{
    return m_paintedScale;
}

qreal ImageEditCanvas::regionX() const
{
    return m_regionOrigin.x();
}

qreal ImageEditCanvas::regionY() const
{
    return m_regionOrigin.y();
}

void ImageEditCanvas::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) {
        refresh();
    }
}

void ImageEditCanvas::refresh()
{
    updatePaintedGeometry();
    update();
}

QRectF ImageEditCanvas::displayRegion() const
{
    if (!m_editModel)
        return QRectF();
    if (m_showWholeImage) {
        return QRectF(0, 0, m_editModel->imageWidth(), m_editModel->imageHeight());
    }
    return m_editModel->cropRect();
}

void ImageEditCanvas::updatePaintedGeometry()
{
    const QRectF region = displayRegion();
    const QRectF area = boundingRect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
    QRectF painted;
    qreal scale = 1.0;
    if (!region.isEmpty() && !area.isEmpty()) {
        scale = qMin(area.width() / region.width(), area.height() / region.height());
        const QSizeF size(region.width() * scale, region.height() * scale);
        painted =
                QRectF(area.x() + (area.width() - size.width()) / 2,
                       area.y() + (area.height() - size.height()) / 2, size.width(), size.height());
    }
    if (painted == m_paintedRect && qFuzzyCompare(scale, m_paintedScale)
        && region.topLeft() == m_regionOrigin)
        return;
    m_paintedRect = painted;
    m_paintedScale = scale;
    m_regionOrigin = region.topLeft();
    emit paintedGeometryChanged();
}
