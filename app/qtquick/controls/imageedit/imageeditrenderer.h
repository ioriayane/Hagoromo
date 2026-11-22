#ifndef IMAGEEDITRENDERER_H
#define IMAGEEDITRENDERER_H

#include <QColor>
#include <QImage>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QString>

// 画像に重ねる編集（下から順に描画する）
struct ImageEditLayer
{
    enum Type {
        Mosaic = 0,
        Line = 1,
        DrawRectangle = 2,
        DrawEllipse = 3,
        Freehand = 4,
    };
    enum Shape {
        Rectangle = 0,
        Ellipse = 1,
    };

    int type = Mosaic;
    int shape = Rectangle; // Mosaicの形
    QRect rect; // Mosaic, DrawRectangle, DrawEllipse
    int strength = 5; // Mosaicの粗さ（1〜10）
    QList<QPointF> points; // Line（2点）, Freehand
    QColor color = QColor(Qt::red); // Line, DrawRectangle, DrawEllipse, Freehand
    int lineWidth = 3; // 線の太さ（1〜20）
    bool fill = false; // DrawRectangle, DrawEllipse

    bool isDrawing() const { return type != Mosaic; }
    bool isPointBased() const { return type == Line || type == Freehand; }
    // 範囲（回転後の原寸座標）。点で表すレイヤーは点を囲む矩形
    QRectF bounds() const;

    bool operator==(const ImageEditLayer &other) const
    {
        return type == other.type && shape == other.shape && rect == other.rect
                && strength == other.strength && points == other.points && color == other.color
                && lineWidth == other.lineWidth && fill == other.fill;
    }
    bool operator!=(const ImageEditLayer &other) const { return !(*this == other); }
};

// 編集内容
// 座標はすべて「回転を適用した後の原寸画像」を基準にする
// 適用順は 回転 -> レイヤー -> クロップ
struct ImageEditState
{
    int rotation = 0; // 時計回り 0, 90, 180, 270
    QRect crop; // 空のときは全体
    QList<ImageEditLayer> layers;

    bool operator==(const ImageEditState &other) const
    {
        return rotation == other.rotation && crop == other.crop && layers == other.layers;
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
    static QPointF rotatePoint90(const QPointF &point, const QSize &size, bool clockwise);

    // 編集内容を原寸画像に適用する
    static QImage render(const QImage &image, const ImageEditState &state);
    // 回転済みの画像にレイヤーを描画する
    // scale: 回転後の原寸座標からimageの座標への倍率（プレビュー用）
    // long_edge: 原寸画像の長辺（モザイクの粗さの基準）
    static void renderLayers(QImage &image, const QList<ImageEditLayer> &layers, qreal scale,
                             int long_edge);
    // モザイクのブロックの大きさ（原寸座標）
    static int mosaicBlockSize(int strength, int long_edge);
    static void applyMosaic(QImage &image, const QRectF &rect, int shape, int block_size);
    // 線の太さ（原寸座標）
    static qreal lineWidthPixels(int line_width, int long_edge);
    static void drawLayer(QImage &image, const ImageEditLayer &layer, qreal scale, int long_edge);
    // 点がレイヤーの範囲内か（線は太さの半分 + toleranceまで）
    static bool contains(const ImageEditLayer &layer, const QPointF &point, qreal tolerance = 0,
                         int long_edge = 0);

    // 編集済み画像の保存先
    static QString editedFolder();
    static bool isEditedFile(const QString &path);
    // 透過がある場合はPNG、それ以外はJPEGで保存して、保存したファイルパスを返す（失敗は空）
    static QString save(const QImage &image, const QString &folder = QString());
};

#endif // IMAGEEDITRENDERER_H
