#include "imageeditmodel.h"

#include <QFileInfo>
#include <QUrl>

#define PREVIEW_MAX_LENGTH 2048
#define CROP_MIN_LENGTH 8
#define LAYER_MIN_LENGTH 4

ImageEditModel::ImageEditModel(QObject *parent)
    : QObject { parent },
      m_editable(false),
      m_previewScale(1.0),
      m_selectedLayer(-1),
      m_transient(false)
{
}

void ImageEditModel::rotateLeft()
{
    if (!editable())
        return;
    const QSize size = imageSize();
    ImageEditState state = m_state;
    state.rotation = ImageEditRenderer::normalizeRotation(m_state.rotation - 90);
    state.crop = ImageEditRenderer::rotateRect90(m_state.crop, size, false);
    for (auto &layer : state.layers) {
        layer.rect = ImageEditRenderer::rotateRect90(layer.rect, size, false);
    }
    applyState(state, true);
}

void ImageEditModel::rotateRight()
{
    if (!editable())
        return;
    const QSize size = imageSize();
    ImageEditState state = m_state;
    state.rotation = ImageEditRenderer::normalizeRotation(m_state.rotation + 90);
    state.crop = ImageEditRenderer::rotateRect90(m_state.crop, size, true);
    for (auto &layer : state.layers) {
        layer.rect = ImageEditRenderer::rotateRect90(layer.rect, size, true);
    }
    applyState(state, true);
}

void ImageEditModel::setCrop(qreal x, qreal y, qreal width, qreal height)
{
    if (!editable())
        return;
    const QRect bounds(QPoint(0, 0), imageSize());
    QRect crop = QRectF(x, y, width, height).toRect().normalized().intersected(bounds);
    if (crop.width() < CROP_MIN_LENGTH || crop.height() < CROP_MIN_LENGTH) {
        // 小さすぎる指定は無視する
        return;
    }
    ImageEditState state = m_state;
    state.crop = (crop == bounds) ? QRect() : crop;
    applyState(state, true);
}

void ImageEditModel::resetCrop()
{
    if (!editable())
        return;
    ImageEditState state = m_state;
    state.crop = QRect();
    applyState(state, true);
}

void ImageEditModel::resetAll()
{
    if (!editable())
        return;
    applyState(ImageEditState(), true);
}

void ImageEditModel::undo()
{
    commitTransient();
    if (m_undoStack.isEmpty())
        return;
    m_redoStack.append(m_state);
    ImageEditState state = m_undoStack.takeLast();
    applyState(state, false);
}

void ImageEditModel::redo()
{
    commitTransient();
    if (m_redoStack.isEmpty())
        return;
    m_undoStack.append(m_state);
    ImageEditState state = m_redoStack.takeLast();
    applyState(state, false);
}

void ImageEditModel::clear()
{
    setSource(QString());
}

int ImageEditModel::addMosaic(qreal x, qreal y, qreal width, qreal height, int shape, int strength)
{
    if (!editable())
        return -1;
    const QRect rect = validLayerRect(x, y, width, height);
    if (rect.isNull())
        return -1;
    commitTransient();

    ImageEditLayer layer;
    layer.type = ImageEditLayer::Mosaic;
    layer.shape = (shape == ImageEditLayer::Ellipse) ? ImageEditLayer::Ellipse
                                                     : ImageEditLayer::Rectangle;
    layer.rect = rect;
    layer.strength = qBound(1, strength, 10);

    ImageEditState state = m_state;
    state.layers.append(layer);
    m_selectedLayer = state.layers.count() - 1;
    applyState(state, true);
    return m_selectedLayer;
}

int ImageEditModel::layerAt(qreal x, qreal y) const
{
    const QPointF point(x, y);
    for (int i = m_state.layers.count() - 1; i >= 0; i--) {
        if (ImageEditRenderer::contains(m_state.layers.at(i), point)) {
            return i;
        }
    }
    return -1;
}

void ImageEditModel::setLayerRect(int index, qreal x, qreal y, qreal width, qreal height,
                                  bool transient)
{
    if (index < 0 || index >= m_state.layers.count())
        return;
    const QRect rect = validLayerRect(x, y, width, height);
    if (rect.isNull())
        return;
    ImageEditState state = m_state;
    state.layers[index].rect = rect;
    applyEdit(state, transient);
}

void ImageEditModel::setLayerShape(int index, int shape)
{
    if (index < 0 || index >= m_state.layers.count())
        return;
    ImageEditState state = m_state;
    state.layers[index].shape = (shape == ImageEditLayer::Ellipse) ? ImageEditLayer::Ellipse
                                                                   : ImageEditLayer::Rectangle;
    applyEdit(state, false);
}

void ImageEditModel::setLayerStrength(int index, int strength, bool transient)
{
    if (index < 0 || index >= m_state.layers.count())
        return;
    ImageEditState state = m_state;
    state.layers[index].strength = qBound(1, strength, 10);
    applyEdit(state, transient);
}

void ImageEditModel::removeLayer(int index)
{
    if (index < 0 || index >= m_state.layers.count())
        return;
    commitTransient();
    ImageEditState state = m_state;
    state.layers.removeAt(index);
    if (m_selectedLayer == index) {
        m_selectedLayer = -1;
    } else if (m_selectedLayer > index) {
        m_selectedLayer--;
    }
    applyState(state, true);
}

void ImageEditModel::commitTransient()
{
    if (!m_transient)
        return;
    m_transient = false;
    if (m_transientBase != m_state) {
        pushHistory(m_transientBase);
        emit stateChanged();
    }
}

QString ImageEditModel::exportImage(const QString &folder) const
{
    if (m_image.isNull())
        return QString();
    QString path = ImageEditRenderer::save(ImageEditRenderer::render(m_image, m_state), folder);
    if (path.isEmpty())
        return QString();
    return QUrl::fromLocalFile(path).toString();
}

QString ImageEditModel::source() const
{
    return m_source;
}

void ImageEditModel::setSource(const QString &newSource)
{
    if (m_source == newSource)
        return;
    m_source = newSource;

    QUrl url(m_source);
    QString path = url.isLocalFile() ? url.toLocalFile() : m_source;
    m_image = m_source.isEmpty() ? QImage() : ImageEditRenderer::load(path);
    // gifはアニメーションを維持するため編集しない
    m_editable = !m_image.isNull() && QFileInfo(path).suffix().toLower() != "gif";

    if (m_image.isNull()) {
        m_preview = QImage();
        m_previewScale = 1.0;
    } else if (qMax(m_image.width(), m_image.height()) > PREVIEW_MAX_LENGTH) {
        m_preview = m_image.scaled(PREVIEW_MAX_LENGTH, PREVIEW_MAX_LENGTH, Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
        m_previewScale = static_cast<qreal>(m_preview.width()) / m_image.width();
    } else {
        m_preview = m_image;
        m_previewScale = 1.0;
    }

    m_state = ImageEditState();
    m_undoStack.clear();
    m_redoStack.clear();
    m_selectedLayer = -1;
    m_transient = false;
    updatePreview(true);

    emit sourceChanged();
    emit stateChanged();
}

bool ImageEditModel::ready() const
{
    return !m_image.isNull();
}

bool ImageEditModel::editable() const
{
    return m_editable;
}

int ImageEditModel::rotation() const
{
    return m_state.rotation;
}

int ImageEditModel::imageWidth() const
{
    return imageSize().width();
}

int ImageEditModel::imageHeight() const
{
    return imageSize().height();
}

QSize ImageEditModel::imageSize() const
{
    return ImageEditRenderer::rotatedSize(m_image.size(), m_state.rotation);
}

QRectF ImageEditModel::cropRect() const
{
    if (m_state.crop.isNull())
        return QRectF(QPointF(0, 0), QSizeF(imageSize()));
    return QRectF(m_state.crop);
}

bool ImageEditModel::canUndo() const
{
    return !m_undoStack.isEmpty() || (m_transient && m_transientBase != m_state);
}

bool ImageEditModel::canRedo() const
{
    return !m_redoStack.isEmpty();
}

bool ImageEditModel::modified() const
{
    return m_state != ImageEditState();
}

int ImageEditModel::layerCount() const
{
    return m_state.layers.count();
}

int ImageEditModel::selectedLayer() const
{
    return m_selectedLayer;
}

void ImageEditModel::selectLayer(int index)
{
    if (index < -1 || index >= m_state.layers.count())
        index = -1;
    if (m_selectedLayer == index)
        return;
    commitTransient();
    m_selectedLayer = index;
    emit stateChanged();
}

QRectF ImageEditModel::selectedLayerRect() const
{
    if (m_selectedLayer < 0 || m_selectedLayer >= m_state.layers.count())
        return QRectF();
    return QRectF(m_state.layers.at(m_selectedLayer).rect);
}

int ImageEditModel::selectedLayerShape() const
{
    if (m_selectedLayer < 0 || m_selectedLayer >= m_state.layers.count())
        return -1;
    return m_state.layers.at(m_selectedLayer).shape;
}

int ImageEditModel::selectedLayerStrength() const
{
    if (m_selectedLayer < 0 || m_selectedLayer >= m_state.layers.count())
        return -1;
    return m_state.layers.at(m_selectedLayer).strength;
}

QList<ImageEditLayer> ImageEditModel::layers() const
{
    return m_state.layers;
}

QImage ImageEditModel::previewImage() const
{
    return m_previewRendered;
}

qreal ImageEditModel::previewScale() const
{
    return m_previewScale;
}

void ImageEditModel::applyState(const ImageEditState &state, bool record_history)
{
    if (state == m_state)
        return;
    if (record_history) {
        pushHistory(m_state);
    }
    const bool rotation_changed = (state.rotation != m_state.rotation);
    const bool layers_changed = (state.layers != m_state.layers);
    m_state = state;
    if (m_selectedLayer >= m_state.layers.count()) {
        m_selectedLayer = -1;
    }
    if (rotation_changed || layers_changed) {
        updatePreview(rotation_changed);
    }
    emit stateChanged();
}

void ImageEditModel::applyEdit(const ImageEditState &state, bool transient)
{
    if (transient) {
        if (!m_transient) {
            m_transient = true;
            m_transientBase = m_state;
        }
        applyState(state, false);
    } else if (m_transient) {
        // 一時的な変更の確定
        applyState(state, false);
        commitTransient();
    } else {
        applyState(state, true);
    }
}

void ImageEditModel::pushHistory(const ImageEditState &state)
{
    m_undoStack.append(state);
    m_redoStack.clear();
}

void ImageEditModel::updatePreview(bool rotation_changed)
{
    if (rotation_changed) {
        m_previewRotated = ImageEditRenderer::rotate(m_preview, m_state.rotation);
    }
    m_previewRendered = m_previewRotated;
    ImageEditRenderer::renderLayers(m_previewRendered, m_state.layers, m_previewScale, longEdge());
    emit previewChanged();
}

QRect ImageEditModel::validLayerRect(qreal x, qreal y, qreal width, qreal height) const
{
    const QRect bounds(QPoint(0, 0), imageSize());
    const QRect rect = QRectF(x, y, width, height).toRect().normalized();
    const QRect visible = rect.intersected(bounds);
    if (visible.width() < LAYER_MIN_LENGTH || visible.height() < LAYER_MIN_LENGTH) {
        return QRect();
    }
    // 楕円の形を保つため画像外へのはみ出しは許容する（描画時に切り取る）
    return rect;
}

int ImageEditModel::longEdge() const
{
    return qMax(m_image.width(), m_image.height());
}
