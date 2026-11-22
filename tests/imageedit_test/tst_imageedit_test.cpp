#include <QtTest>
#include <QCoreApplication>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "content/imagecompressor.h"
#include "controls/embedimagelistmodel.h"
#include "controls/imageedit/imageeditmodel.h"
#include "controls/imageedit/imageeditrenderer.h"

class imageedit_test : public QObject
{
    Q_OBJECT

public:
    imageedit_test();
    ~imageedit_test();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_ImageCompressor_stripExif();
    void test_ImageCompressor_keepFormat();
    void test_ImageEditRenderer_rotateRect90();
    void test_ImageEditModel_rotateAndCrop();
    void test_ImageEditModel_export();
    void test_EmbedImageListModel_editedFiles();

private:
    // 左半分が赤、右半分が青の画像
    QImage makeHalfImage(int width, int height, QImage::Format format) const;
    QByteArray makeExifSegment() const;
    QString compressSync(const QString &path, bool keep_format, QSize *size = nullptr) const;
    bool isReddish(const QColor &color) const;
    bool isBluish(const QColor &color) const;

    QTemporaryDir m_tempDir;
};

imageedit_test::imageedit_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));
    QStandardPaths::setTestModeEnabled(true);
}

imageedit_test::~imageedit_test() { }

void imageedit_test::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    QDir(ImageEditRenderer::editedFolder()).removeRecursively();
}

void imageedit_test::cleanupTestCase()
{
    QDir(ImageEditRenderer::editedFolder()).removeRecursively();
}

void imageedit_test::test_ImageCompressor_stripExif()
{
    // EXIF（回転とGPS）とコメントを含むJPEGを作る
    QImage src = makeHalfImage(40, 20, QImage::Format_RGB32);
    src.setText(QStringLiteral("Description"), QStringLiteral("secret-comment"));
    QByteArray jpeg;
    {
        QBuffer buffer(&jpeg);
        buffer.open(QIODevice::WriteOnly);
        QVERIFY(src.save(&buffer, "JPEG", 95));
    }
    QVERIFY(jpeg.startsWith(QByteArray::fromHex("ffd8")));
    jpeg.insert(2, makeExifSegment());
    const QString path = m_tempDir.filePath("exif.jpg");
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(jpeg);
    }
    // 前提の確認
    QVERIFY(jpeg.contains("Exif"));
    QVERIFY(jpeg.contains("secret-comment"));
    {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        QCOMPARE(reader.read().size(), QSize(20, 40));
    }

    QSize size;
    const QString new_path = compressSync(path, false, &size);
    QVERIFY(new_path != path);
    QFile file(new_path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray data = file.readAll();
    file.close();
    QVERIFY(!data.contains("Exif"));
    QVERIFY(!data.contains("secret-comment"));

    // 回転は画素に反映されている（左側が上になる）
    QCOMPARE(size, QSize(20, 40));
    QImage result(new_path);
    QCOMPARE(result.size(), QSize(20, 40));
    QVERIFY(isReddish(result.pixelColor(10, 5)));
    QVERIFY(isBluish(result.pixelColor(10, 35)));
    QFile::remove(new_path);
}

void imageedit_test::test_ImageCompressor_keepFormat()
{
    QImage src(20, 20, QImage::Format_ARGB32);
    src.fill(Qt::transparent);
    src.setPixelColor(5, 5, QColor(255, 0, 0));
    src.setText(QStringLiteral("Comment"), QStringLiteral("secret-png"));
    const QString path = m_tempDir.filePath("text.png");
    QVERIFY(src.save(path, "PNG"));
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(file.readAll().contains("secret-png"));
    }

    const QString new_path = compressSync(path, true);
    QVERIFY(new_path != path);
    QVERIFY(new_path.endsWith(".png"));
    QFile file(new_path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY(!file.readAll().contains("secret-png"));
    file.close();

    QImage result(new_path);
    QVERIFY(result.hasAlphaChannel());
    QCOMPARE(result.pixelColor(0, 0).alpha(), 0);
    QCOMPARE(result.pixelColor(5, 5), QColor(255, 0, 0));
    QFile::remove(new_path);
}

void imageedit_test::test_ImageEditRenderer_rotateRect90()
{
    const QSize size(30, 20);
    const QRect rect(2, 3, 5, 4);
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::white);
    {
        QPainter painter(&image);
        painter.fillRect(rect, Qt::red);
    }

    const bool directions[] = { true, false };
    for (const bool clockwise : directions) {
        const QImage rotated = ImageEditRenderer::rotate(image, clockwise ? 90 : -90);
        QCOMPARE(rotated.size(), size.transposed());
        QRect found;
        for (int y = 0; y < rotated.height(); y++) {
            for (int x = 0; x < rotated.width(); x++) {
                if (rotated.pixelColor(x, y) == QColor(Qt::red)) {
                    found |= QRect(x, y, 1, 1);
                }
            }
        }
        QCOMPARE(ImageEditRenderer::rotateRect90(rect, size, clockwise), found);
        // 逆回転で元に戻る
        QCOMPARE(ImageEditRenderer::rotateRect90(found, size.transposed(), !clockwise), rect);
    }

    QCOMPARE(ImageEditRenderer::normalizeRotation(-90), 270);
    QCOMPARE(ImageEditRenderer::normalizeRotation(450), 90);
    QCOMPARE(ImageEditRenderer::rotatedSize(size, 270), size.transposed());
    QCOMPARE(ImageEditRenderer::rotatedSize(size, 180), size);
}

void imageedit_test::test_ImageEditModel_rotateAndCrop()
{
    const QString path = m_tempDir.filePath("model.png");
    QVERIFY(makeHalfImage(40, 20, QImage::Format_RGB32).save(path, "PNG"));

    ImageEditModel model;
    QSignalSpy spy(&model, &ImageEditModel::stateChanged);
    model.setSource(QUrl::fromLocalFile(path).toString());
    QVERIFY(model.ready());
    QVERIFY(model.editable());
    QVERIFY(!model.modified());
    QCOMPARE(model.imageWidth(), 40);
    QCOMPARE(model.imageHeight(), 20);
    QCOMPARE(model.cropRect(), QRectF(0, 0, 40, 20));

    // 赤い左半分をクロップ
    model.setCrop(0, 0, 20, 20);
    QVERIFY(model.modified());
    QCOMPARE(model.cropRect(), QRectF(0, 0, 20, 20));

    // 回転してもクロップ範囲は同じ部分を指す
    model.rotateRight();
    QCOMPARE(model.rotation(), 90);
    QCOMPARE(model.imageWidth(), 20);
    QCOMPARE(model.imageHeight(), 40);
    QCOMPARE(model.cropRect(), QRectF(0, 0, 20, 20));
    model.rotateLeft();
    model.rotateLeft();
    QCOMPARE(model.rotation(), 270);
    QCOMPARE(model.cropRect(), QRectF(0, 20, 20, 20));

    // 小さすぎる範囲は無視、全体は未指定扱い
    model.setCrop(0, 0, 3, 3);
    QCOMPARE(model.cropRect(), QRectF(0, 20, 20, 20));
    model.setCrop(-10, -10, 100, 100);
    QCOMPARE(model.cropRect(), QRectF(0, 0, 20, 40));
    QCOMPARE(model.rotation(), 270);

    // undo / redo
    QVERIFY(model.canUndo());
    QVERIFY(!model.canRedo());
    model.undo(); // 2回目の左回転の後
    QCOMPARE(model.rotation(), 270);
    QCOMPARE(model.cropRect(), QRectF(0, 20, 20, 20));
    model.undo(); // 1回目の左回転の後
    QCOMPARE(model.rotation(), 0);
    model.undo(); // 右回転の後
    QCOMPARE(model.rotation(), 90);
    model.undo(); // クロップの後
    QCOMPARE(model.rotation(), 0);
    QCOMPARE(model.cropRect(), QRectF(0, 0, 20, 20));
    model.undo(); // 未編集
    QVERIFY(!model.canUndo());
    QVERIFY(!model.modified());
    QCOMPARE(model.cropRect(), QRectF(0, 0, 40, 20));
    model.redo();
    QCOMPARE(model.cropRect(), QRectF(0, 0, 20, 20));
    QVERIFY(model.canRedo());

    // 新しい編集でredoは消える
    model.resetAll();
    QVERIFY(!model.canRedo());
    QVERIFY(!model.modified());
    QVERIFY(model.canUndo());

    // 画像を変えると履歴は消える
    model.clear();
    QVERIFY(!model.ready());
    QVERIFY(!model.canUndo());
    QVERIFY(spy.count() > 0);
}

void imageedit_test::test_ImageEditModel_export()
{
    const QString path = m_tempDir.filePath("export.png");
    QVERIFY(makeHalfImage(40, 20, QImage::Format_RGB32).save(path, "PNG"));

    ImageEditModel model;
    model.setSource(QUrl::fromLocalFile(path).toString());
    model.rotateRight();
    model.setCrop(0, 0, 20, 30);

    const QString url = model.exportImage();
    QVERIFY(!url.isEmpty());
    const QString exported = QUrl(url).toLocalFile();
    QVERIFY(ImageEditRenderer::isEditedFile(exported));
    QVERIFY(exported.endsWith(".jpg"));
    QImage result(exported);
    QCOMPARE(result.size(), QSize(20, 30));
    QVERIFY(isReddish(result.pixelColor(10, 5)));
    QVERIFY(isBluish(result.pixelColor(10, 25)));
    QFile::remove(exported);

    // 透過がある場合はPNG
    QImage alpha(10, 10, QImage::Format_ARGB32);
    alpha.fill(Qt::transparent);
    const QString alpha_path = m_tempDir.filePath("alpha.png");
    QVERIFY(alpha.save(alpha_path, "PNG"));
    model.setSource(QUrl::fromLocalFile(alpha_path).toString());
    model.rotateRight();
    const QString alpha_exported = QUrl(model.exportImage()).toLocalFile();
    QVERIFY(alpha_exported.endsWith(".png"));
    QCOMPARE(QImage(alpha_exported).pixelColor(0, 0).alpha(), 0);
    QFile::remove(alpha_exported);

    // 元ファイルは変更しない
    QCOMPARE(QImage(path).size(), QSize(40, 20));
}

void imageedit_test::test_EmbedImageListModel_editedFiles()
{
    const QString original = m_tempDir.filePath("original.png");
    QVERIFY(makeHalfImage(40, 20, QImage::Format_RGB32).save(original, "PNG"));
    const QString original_url = QUrl::fromLocalFile(original).toString();
    const QImage image = makeHalfImage(10, 10, QImage::Format_RGB32);
    auto makeEdited = [&image]() -> QString {
        return QUrl::fromLocalFile(ImageEditRenderer::save(image)).toString();
    };
    auto exists = [](const QString &url) -> bool { return QFile::exists(QUrl(url).toLocalFile()); };

    {
        EmbedImageListModel model;
        QVERIFY(model.append(QStringList() << original_url));
        QTRY_COMPARE(model.count(), 1);
        QTRY_VERIFY(!model.running());

        // 差し替えた古い編集済みファイルは削除される（元画像は残す）
        const QString edited1 = makeEdited();
        const QString edited2 = makeEdited();
        model.replaceImage(0, edited1);
        QCOMPARE(model.uris(), QStringList() << edited1);
        QVERIFY(exists(original_url));
        model.replaceImage(0, edited2);
        QCOMPARE(model.uris(), QStringList() << edited2);
        QVERIFY(!exists(edited1));
        QVERIFY(exists(edited2));

        // 一覧から外すと削除される
        model.remove(0);
        QVERIFY(!exists(edited2));
        QVERIFY(exists(original_url));

        // 下書きに保存したものは残す
        QVERIFY(model.append(QStringList() << original_url));
        QTRY_COMPARE(model.count(), 1);
        QTRY_VERIFY(!model.running());
        const QString edited3 = makeEdited();
        model.replaceImage(0, edited3);
        model.keepEditedFiles();
        model.clear();
        QVERIFY(exists(edited3));
        QFile::remove(QUrl(edited3).toLocalFile());

        // 編集済みファイル以外は管理対象にしない
        QVERIFY(model.append(QStringList() << original_url));
        QTRY_COMPARE(model.count(), 1);
        QTRY_VERIFY(!model.running());
        const QString other = QUrl::fromLocalFile(m_tempDir.filePath("other.png")).toString();
        QVERIFY(image.save(QUrl(other).toLocalFile(), "PNG"));
        model.replaceImage(0, other);
        model.clear();
        QVERIFY(exists(other));
    }

    // 破棄時に残っている編集済みファイルは削除される
    QString edited4;
    {
        EmbedImageListModel model;
        QVERIFY(model.append(QStringList() << original_url));
        QTRY_COMPARE(model.count(), 1);
        QTRY_VERIFY(!model.running());
        edited4 = makeEdited();
        model.replaceImage(0, edited4);
        QVERIFY(exists(edited4));
    }
    QVERIFY(!exists(edited4));
    QVERIFY(exists(original_url));
}

QImage imageedit_test::makeHalfImage(int width, int height, QImage::Format format) const
{
    QImage image(width, height, format);
    QPainter painter(&image);
    painter.fillRect(QRect(0, 0, width / 2, height), QColor(255, 0, 0));
    painter.fillRect(QRect(width / 2, 0, width - width / 2, height), QColor(0, 0, 255));
    painter.end();
    return image;
}

QByteArray imageedit_test::makeExifSegment() const
{
    // ビッグエンディアンのTIFF構造
    auto u16 = [](QByteArray &d, quint16 v) {
        d.append(static_cast<char>((v >> 8) & 0xff));
        d.append(static_cast<char>(v & 0xff));
    };
    auto u32 = [&u16](QByteArray &d, quint32 v) {
        u16(d, static_cast<quint16>((v >> 16) & 0xffff));
        u16(d, static_cast<quint16>(v & 0xffff));
    };
    QByteArray tiff;
    tiff.append("MM");
    u16(tiff, 0x002a);
    u32(tiff, 8);
    // IFD0 (offset 8): Orientation, GPS IFD Pointer
    u16(tiff, 2);
    u16(tiff, 0x0112); // Orientation
    u16(tiff, 3); // SHORT
    u32(tiff, 1);
    u16(tiff, 6); // 時計回りに90度回転して表示
    u16(tiff, 0);
    u16(tiff, 0x8825); // GPS IFD
    u16(tiff, 4); // LONG
    u32(tiff, 1);
    u32(tiff, 38);
    u32(tiff, 0); // next IFD
    // GPS IFD (offset 38): GPSLatitudeRef, GPSLatitude
    u16(tiff, 2);
    u16(tiff, 0x0001);
    u16(tiff, 2); // ASCII
    u32(tiff, 2);
    tiff.append("N\0\0\0", 4);
    u16(tiff, 0x0002);
    u16(tiff, 5); // RATIONAL
    u32(tiff, 3);
    u32(tiff, 68);
    u32(tiff, 0); // next IFD
    // offset 68
    u32(tiff, 35);
    u32(tiff, 1);
    u32(tiff, 39);
    u32(tiff, 1);
    u32(tiff, 0);
    u32(tiff, 1);

    QByteArray segment;
    segment.append(QByteArray::fromHex("ffe1"));
    u16(segment, static_cast<quint16>(2 + 6 + tiff.size()));
    segment.append("Exif\0\0", 6);
    segment.append(tiff);
    return segment;
}

QString imageedit_test::compressSync(const QString &path, bool keep_format, QSize *size) const
{
    ImageCompressor compressor;
    QSignalSpy spy(&compressor, &ImageCompressor::compressed);
    compressor.compress(path, 0, keep_format);
    if (spy.count() != 1)
        return QString();
    const QList<QVariant> arguments = spy.takeFirst();
    if (size) {
        *size = arguments.at(1).toSize();
    }
    return arguments.at(0).toString();
}

bool imageedit_test::isReddish(const QColor &color) const
{
    return color.red() > 200 && color.green() < 60 && color.blue() < 60;
}

bool imageedit_test::isBluish(const QColor &color) const
{
    return color.red() < 60 && color.green() < 60 && color.blue() > 200;
}

QTEST_MAIN(imageedit_test)

#include "tst_imageedit_test.moc"
