#ifndef IMAGEEDITMODEL_H
#define IMAGEEDITMODEL_H

#include "imageeditrenderer.h"

#include <QColor>
#include <QObject>
#include <QVariantList>
#include <QImage>
#include <QList>
#include <QRectF>
#include <QSize>
#include <QThreadPool>

// 投稿する画像の編集状態を管理する（非破壊編集）
// 元画像は変更せず、exportImage()で編集結果を別ファイルとして書き出す
class ImageEditModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(bool ready READ ready NOTIFY sourceChanged FINAL)
    Q_PROPERTY(bool editable READ editable NOTIFY sourceChanged FINAL)
    // 画像を読み込み中（読み込みはバックグラウンドで行う）
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged FINAL)
    Q_PROPERTY(int rotation READ rotation NOTIFY stateChanged FINAL)
    // 回転後の原寸サイズ
    Q_PROPERTY(int imageWidth READ imageWidth NOTIFY stateChanged FINAL)
    Q_PROPERTY(int imageHeight READ imageHeight NOTIFY stateChanged FINAL)
    // 回転後の原寸座標でのクロップ範囲（未指定なら画像全体）
    Q_PROPERTY(QRectF cropRect READ cropRect NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool modified READ modified NOTIFY stateChanged FINAL)
    // レイヤー
    Q_PROPERTY(int layerCount READ layerCount NOTIFY stateChanged FINAL)
    Q_PROPERTY(int selectedLayer READ selectedLayer WRITE selectLayer NOTIFY stateChanged FINAL)
    Q_PROPERTY(QRectF selectedLayerRect READ selectedLayerRect NOTIFY stateChanged FINAL)
    Q_PROPERTY(int selectedLayerShape READ selectedLayerShape NOTIFY stateChanged FINAL)
    Q_PROPERTY(int selectedLayerStrength READ selectedLayerStrength NOTIFY stateChanged FINAL)
    Q_PROPERTY(int selectedLayerType READ selectedLayerType NOTIFY stateChanged FINAL)
    Q_PROPERTY(QColor selectedLayerColor READ selectedLayerColor NOTIFY stateChanged FINAL)
    Q_PROPERTY(int selectedLayerLineWidth READ selectedLayerLineWidth NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool selectedLayerFill READ selectedLayerFill NOTIFY stateChanged FINAL)
    Q_PROPERTY(QVariantList selectedLayerPoints READ selectedLayerPoints NOTIFY stateChanged FINAL)
    Q_PROPERTY(int selectedLayerOpacity READ selectedLayerOpacity NOTIFY stateChanged FINAL)

public:
    explicit ImageEditModel(QObject *parent = nullptr);
    ~ImageEditModel();

    enum LayerShape {
        ShapeRectangle = ImageEditLayer::Rectangle,
        ShapeEllipse = ImageEditLayer::Ellipse,
    };
    Q_ENUM(LayerShape)
    enum LayerType {
        TypeMosaic = ImageEditLayer::Mosaic,
        TypeLine = ImageEditLayer::Line,
        TypeRectangle = ImageEditLayer::DrawRectangle,
        TypeEllipse = ImageEditLayer::DrawEllipse,
        TypeFreehand = ImageEditLayer::Freehand,
        TypeStamp = ImageEditLayer::Stamp,
        TypeArrow = ImageEditLayer::Arrow,
    };
    Q_ENUM(LayerType)
    // layerAt()で対象にするレイヤーの種類
    enum LayerCategory {
        CategoryAll = 0,
        CategoryMosaic = 1,
        CategoryDrawing = 2,
        CategoryStamp = 3,
    };
    Q_ENUM(LayerCategory)

    Q_INVOKABLE void rotateLeft();
    Q_INVOKABLE void rotateRight();
    Q_INVOKABLE void setCrop(qreal x, qreal y, qreal width, qreal height);
    Q_INVOKABLE void resetCrop();
    Q_INVOKABLE void resetAll();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void clear();

    // モザイクを追加して、追加したレイヤーを選択する。追加できなかったときは-1
    Q_INVOKABLE int addMosaic(qreal x, qreal y, qreal width, qreal height, int shape, int strength);
    // 図形（TypeRectangle / TypeEllipse）を追加して選択する。追加できなかったときは-1
    Q_INVOKABLE int addShape(int type, qreal x, qreal y, qreal width, qreal height,
                             const QColor &color, int line_width, bool fill);
    // 直線を追加して選択する。追加できなかったときは-1
    Q_INVOKABLE int addLine(qreal x1, qreal y1, qreal x2, qreal y2, const QColor &color,
                            int line_width);
    // 矢印（終点側に三角形）を追加して選択する。追加できなかったときは-1
    Q_INVOKABLE int addArrow(qreal x1, qreal y1, qreal x2, qreal y2, const QColor &color,
                             int line_width);
    // フリー描画（点の配列）を追加して選択する。追加できなかったときは-1
    Q_INVOKABLE int addFreehand(const QVariantList &points, const QColor &color, int line_width);
    // 画像（スタンプ）を追加して選択する。追加できなかったときは-1
    // 中心と長辺の長さ（回転後の原寸座標）を指定し、縦横比は画像に合わせる
    Q_INVOKABLE int addStamp(const QString &url, qreal center_x, qreal center_y, qreal max_length);
    // 既存のスタンプと中心が重ならない位置を返す
    // 中心からstepずつ右下へずらし、areaの外に出るときはareaの左上へ折り返す
    Q_INVOKABLE QPointF findStampPosition(qreal center_x, qreal center_y, qreal step,
                                          const QRectF &area) const;
    // 指定位置（回転後の原寸座標）にある一番上のレイヤー。無ければ-1
    // tolerance: 線の判定の余裕（原寸座標）
    Q_INVOKABLE int layerAt(qreal x, qreal y, qreal tolerance = 0, int category = 0) const;
    // 一時的な変更を始める（以降の変更はcommitTransient()で1回分の履歴になる）
    Q_INVOKABLE void beginTransaction();
    // transient=trueのときは履歴に残さずに反映する（ドラッグ中など）
    // その後のtransient=falseの変更かcommitTransient()で、まとめて1回分の履歴にする
    // 点で表すレイヤーは点を拡大縮小・移動して矩形に合わせる
    Q_INVOKABLE void setLayerRect(int index, qreal x, qreal y, qreal width, qreal height,
                                  bool transient = false);
    Q_INVOKABLE void setLayerShape(int index, int shape);
    Q_INVOKABLE void setLayerStrength(int index, int strength, bool transient = false);
    Q_INVOKABLE void setLayerPoints(int index, const QVariantList &points, bool transient = false);
    Q_INVOKABLE void setLayerColor(int index, const QColor &color);
    Q_INVOKABLE void setLayerLineWidth(int index, int line_width, bool transient = false);
    Q_INVOKABLE void setLayerFill(int index, bool fill);
    Q_INVOKABLE void setLayerOpacity(int index, int opacity, bool transient = false);
    Q_INVOKABLE void removeLayer(int index);
    Q_INVOKABLE void commitTransient();

    // 編集結果を保存してファイルのURLを返す。失敗時は空文字
    Q_INVOKABLE QString exportImage(const QString &folder = QString()) const;

    QString source() const;
    void setSource(const QString &newSource);
    bool ready() const;
    bool loading() const;
    bool editable() const;
    int rotation() const;
    int imageWidth() const;
    int imageHeight() const;
    QSize imageSize() const;
    QRectF cropRect() const;
    bool canUndo() const;
    bool canRedo() const;
    bool modified() const;
    int layerCount() const;
    int selectedLayer() const;
    void selectLayer(int index);
    QRectF selectedLayerRect() const;
    int selectedLayerShape() const;
    int selectedLayerStrength() const;
    int selectedLayerType() const;
    QColor selectedLayerColor() const;
    int selectedLayerLineWidth() const;
    bool selectedLayerFill() const;
    QVariantList selectedLayerPoints() const;
    int selectedLayerOpacity() const;
    QList<ImageEditLayer> layers() const;

    // 表示用（長辺を制限した縮小画像に回転とレイヤーを適用したもの）
    QImage previewImage() const;
    // 回転後の原寸座標から表示用画像の座標への倍率
    qreal previewScale() const;

signals:
    void sourceChanged();
    void stateChanged();
    void previewChanged();
    void loadingChanged();

private:
    void applyState(const ImageEditState &state, bool record_history);
    void applyEdit(const ImageEditState &state, bool transient);
    void pushHistory(const ImageEditState &state);
    int addLayer(const ImageEditLayer &layer);
    int addTwoPointLayer(int type, qreal x1, qreal y1, qreal x2, qreal y2, const QColor &color,
                         int line_width);
    void rotateState(bool clockwise);
    const ImageEditLayer *selectedLayerData() const;
    static QList<QPointF> toPoints(const QVariantList &points);
    void updatePreview(bool rotation_changed);
    void setLoading(bool loading);
    void finishLoading(int load_id, const QImage &image, const QImage &preview, qreal scale,
                       bool editable);
    QRect validLayerRect(qreal x, qreal y, qreal width, qreal height) const;
    int longEdge() const;

    QString m_source;
    bool m_editable;
    QImage m_image; // 原寸（EXIF回転適用済み・回転編集前）
    QImage m_preview; // 縮小（回転編集前）
    QImage m_previewRotated; // 縮小（回転編集後）
    QImage m_previewRendered; // 縮小（回転編集とレイヤー適用後）
    qreal m_previewScale;
    ImageEditState m_state;
    QList<ImageEditState> m_undoStack;
    QList<ImageEditState> m_redoStack;
    int m_selectedLayer;
    bool m_loading;
    int m_loadId; // 古い読み込み結果を捨てるための番号
    QThreadPool m_loadPool;
    bool m_transient;
    ImageEditState m_transientBase; // 一時的な変更を始める前の状態
};

#endif // IMAGEEDITMODEL_H
