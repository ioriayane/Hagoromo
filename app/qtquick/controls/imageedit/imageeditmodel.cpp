#include "imageeditmodel.h"

#include <QFileInfo>
#include <QUrl>

#define PREVIEW_MAX_LENGTH 2048
#define CROP_MIN_LENGTH 8

ImageEditModel::ImageEditModel(QObject *parent)
    : QObject { parent }, m_editable(false), m_previewScale(1.0)
{
}

void ImageEditModel::rotateLeft()
{
    if (!editable())
        return;
    ImageEditState state = m_state;
    state.rotation = ImageEditRenderer::normalizeRotation(m_state.rotation - 90);
    state.crop = ImageEditRenderer::rotateRect90(m_state.crop, imageSize(), false);
    applyState(state, true);
}

void ImageEditModel::rotateRight()
{
    if (!editable())
        return;
    ImageEditState state = m_state;
    state.rotation = ImageEditRenderer::normalizeRotation(m_state.rotation + 90);
    state.crop = ImageEditRenderer::rotateRect90(m_state.crop, imageSize(), true);
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
    if (m_undoStack.isEmpty())
        return;
    m_redoStack.append(m_state);
    ImageEditState state = m_undoStack.takeLast();
    applyState(state, false);
}

void ImageEditModel::redo()
{
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
    updatePreview();

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
    return !m_undoStack.isEmpty();
}

bool ImageEditModel::canRedo() const
{
    return !m_redoStack.isEmpty();
}

bool ImageEditModel::modified() const
{
    return m_state != ImageEditState();
}

QImage ImageEditModel::previewImage() const
{
    return m_previewRotated;
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
        m_undoStack.append(m_state);
        m_redoStack.clear();
    }
    const bool rotation_changed = (state.rotation != m_state.rotation);
    m_state = state;
    if (rotation_changed) {
        updatePreview();
    }
    emit stateChanged();
}

void ImageEditModel::updatePreview()
{
    m_previewRotated = ImageEditRenderer::rotate(m_preview, m_state.rotation);
    emit previewChanged();
}
