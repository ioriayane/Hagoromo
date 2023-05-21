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
};

#endif // IMAGECOMPRESSOR_H
