#ifndef IMAGEEDITMODEL_H
#define IMAGEEDITMODEL_H

#include "imageeditrenderer.h"

#include <QObject>
#include <QImage>
#include <QList>
#include <QRectF>
#include <QSize>

// 投稿する画像の編集状態を管理する（非破壊編集）
// 元画像は変更せず、exportImage()で編集結果を別ファイルとして書き出す
class ImageEditModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(bool ready READ ready NOTIFY sourceChanged FINAL)
    Q_PROPERTY(bool editable READ editable NOTIFY sourceChanged FINAL)
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

public:
    explicit ImageEditModel(QObject *parent = nullptr);

    enum LayerShape {
        ShapeRectangle = ImageEditLayer::Rectangle,
        ShapeEllipse = ImageEditLayer::Ellipse,
    };
    Q_ENUM(LayerShape)

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
    // 指定位置（回転後の原寸座標）にある一番上のレイヤー。無ければ-1
    Q_INVOKABLE int layerAt(qreal x, qreal y) const;
    // transient=trueのときは履歴に残さずに反映する（ドラッグ中など）
    // その後のtransient=falseの変更かcommitTransient()で、まとめて1回分の履歴にする
    Q_INVOKABLE void setLayerRect(int index, qreal x, qreal y, qreal width, qreal height,
                                  bool transient = false);
    Q_INVOKABLE void setLayerShape(int index, int shape);
    Q_INVOKABLE void setLayerStrength(int index, int strength, bool transient = false);
    Q_INVOKABLE void removeLayer(int index);
    Q_INVOKABLE void commitTransient();

    // 編集結果を保存してファイルのURLを返す。失敗時は空文字
    Q_INVOKABLE QString exportImage(const QString &folder = QString()) const;

    QString source() const;
    void setSource(const QString &newSource);
    bool ready() const;
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
    QList<ImageEditLayer> layers() const;

    // 表示用（長辺を制限した縮小画像に回転とレイヤーを適用したもの）
    QImage previewImage() const;
    // 回転後の原寸座標から表示用画像の座標への倍率
    qreal previewScale() const;

signals:
    void sourceChanged();
    void stateChanged();
    void previewChanged();

private:
    void applyState(const ImageEditState &state, bool record_history);
    void applyEdit(const ImageEditState &state, bool transient);
    void pushHistory(const ImageEditState &state);
    void updatePreview(bool rotation_changed);
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
    bool m_transient;
    ImageEditState m_transientBase; // 一時的な変更を始める前の状態
};

#endif // IMAGEEDITMODEL_H
