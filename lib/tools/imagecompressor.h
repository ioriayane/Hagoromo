#ifndef IMAGECOMPRESSOR_H
#define IMAGECOMPRESSOR_H

#include <QObject>

class ImageCompressor : public QObject
{
    Q_OBJECT
public:
    explicit ImageCompressor(QObject *parent = nullptr);

public slots:
    void compress(const QString &path);

signals:
    void compressed(const QString &path);

private:
    bool save(const QString path, const QImage img, const int quality);
};

#endif // IMAGECOMPRESSOR_H
