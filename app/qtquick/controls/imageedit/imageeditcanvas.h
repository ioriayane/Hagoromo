#ifndef IMAGEEDITCANVAS_H
#define IMAGEEDITCANVAS_H

#include "imageeditmodel.h"

#include <QQuickPaintedItem>
#include <QPointer>

// ImageEditModelの編集結果をプレビュー表示する
// 座標変換用に表示位置と倍率を公開し、操作用のハンドルなどはQML側で重ねる
class ImageEditCanvas : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(ImageEditModel *editModel READ editModel WRITE setEditModel NOTIFY editModelChanged
                       FINAL)
    // true: 回転後の画像全体を表示（クロップ編集用） false: クロップ後の範囲を表示
    Q_PROPERTY(bool showWholeImage READ showWholeImage WRITE setShowWholeImage NOTIFY
                       showWholeImageChanged FINAL)
    Q_PROPERTY(qreal margin READ margin WRITE setMargin NOTIFY marginChanged FINAL)
    // 表示中の画像の位置（アイテム座標）と倍率（アイテム座標/原寸座標）
    Q_PROPERTY(qreal paintedX READ paintedX NOTIFY paintedGeometryChanged FINAL)
    Q_PROPERTY(qreal paintedY READ paintedY NOTIFY paintedGeometryChanged FINAL)
    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedGeometryChanged FINAL)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedGeometryChanged FINAL)
    Q_PROPERTY(qreal paintedScale READ paintedScale NOTIFY paintedGeometryChanged FINAL)

public:
    explicit ImageEditCanvas(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    // アイテム座標 <-> 回転後の原寸画像座標
    Q_INVOKABLE QPointF mapToImage(qreal x, qreal y) const;
    Q_INVOKABLE QPointF mapFromImage(qreal x, qreal y) const;

    ImageEditModel *editModel() const;
    void setEditModel(ImageEditModel *newEditModel);
    bool showWholeImage() const;
    void setShowWholeImage(bool newShowWholeImage);
    qreal margin() const;
    void setMargin(qreal newMargin);
    qreal paintedX() const;
    qreal paintedY() const;
    qreal paintedWidth() const;
    qreal paintedHeight() const;
    qreal paintedScale() const;

signals:
    void editModelChanged();
    void showWholeImageChanged();
    void marginChanged();
    void paintedGeometryChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private slots:
    void refresh();

private:
    QRectF displayRegion() const;
    void updatePaintedGeometry();

    QPointer<ImageEditModel> m_editModel;
    bool m_showWholeImage;
    qreal m_margin;
    QRectF m_paintedRect;
    qreal m_paintedScale;
};

#endif // IMAGEEDITCANVAS_H
