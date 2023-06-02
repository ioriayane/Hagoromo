#include <qquickimageprovider.h>

// Imageのsourceにimage://thumbnail/<FILEPATH>の形式で指定する
// ローカルファイルのみ
class ThumbnailProvider : public QQuickImageProvider
{
public:
    ThumbnailProvider()
        : QQuickImageProvider(QQuickImageProvider::Image,
                              QQuickImageProvider::ForceAsynchronousImageLoading)
    {
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize)
    {
        int width = requestedSize.width() > 0 ? requestedSize.width() : 200;
        int height = requestedSize.height() > 0 ? requestedSize.height() : 120;

        if (size)
            *size = QSize(width, height);

        QImage image(id);
        if (!image.isNull())
            image = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        return image;
    }
};
