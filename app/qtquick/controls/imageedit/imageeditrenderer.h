#ifndef IMAGEEDITRENDERER_H
#define IMAGEEDITRENDERER_H

#include <QImage>
#include <QRect>
#include <QString>

// 編集内容
// 座標はすべて「回転を適用した後の原寸画像」を基準にする
struct ImageEditState
{
    int rotation = 0; // 時計回り 0, 90, 180, 270
    QRect crop; // 空のときは全体

    bool operator==(const ImageEditState &other) const
    {
        return rotation == other.rotation && crop == other.crop;
    }
    bool operator!=(const ImageEditState &other) const { return !(*this == other); }
};

class ImageEditRenderer
{
public:
    // EXIFの回転を適用し、メタデータを除去して読み込む
    static QImage load(const QString &path);

    static int normalizeRotation(int rotation);
    static QSize rotatedSize(const QSize &size, int rotation);
    static QImage rotate(const QImage &image, int rotation);
    // size（回転前の画像サイズ）上の矩形を、時計回りに90度(clockwise=true)
    // または反時計回りに90度回転させた座標系の矩形へ変換する
    static QRect rotateRect90(const QRect &rect, const QSize &size, bool clockwise);

    // 編集内容を原寸画像に適用する
    static QImage render(const QImage &image, const ImageEditState &state);

    // 編集済み画像の保存先
    static QString editedFolder();
    static bool isEditedFile(const QString &path);
    // 透過がある場合はPNG、それ以外はJPEGで保存して、保存したファイルパスを返す（失敗は空）
    static QString save(const QImage &image, const QString &folder = QString());
};

#endif // IMAGEEDITRENDERER_H
