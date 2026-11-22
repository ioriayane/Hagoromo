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

public:
    explicit ImageEditModel(QObject *parent = nullptr);

    Q_INVOKABLE void rotateLeft();
    Q_INVOKABLE void rotateRight();
    Q_INVOKABLE void setCrop(qreal x, qreal y, qreal width, qreal height);
    Q_INVOKABLE void resetCrop();
    Q_INVOKABLE void resetAll();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void clear();

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

    // 表示用（長辺を制限した縮小画像に回転を適用したもの）
    QImage previewImage() const;
    // 回転後の原寸座標から表示用画像の座標への倍率
    qreal previewScale() const;

signals:
    void sourceChanged();
    void stateChanged();
    void previewChanged();

private:
    void applyState(const ImageEditState &state, bool record_history);
    void updatePreview();

    QString m_source;
    bool m_editable;
    QImage m_image; // 原寸（EXIF回転適用済み・回転編集前）
    QImage m_preview; // 縮小（回転編集前）
    QImage m_previewRotated; // 縮小（回転編集後）
    qreal m_previewScale;
    ImageEditState m_state;
    QList<ImageEditState> m_undoStack;
    QList<ImageEditState> m_redoStack;
};

#endif // IMAGEEDITMODEL_H
