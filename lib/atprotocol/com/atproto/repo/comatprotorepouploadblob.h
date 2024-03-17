#ifndef COMATPROTOREPOUPLOADBLOB_H
#define COMATPROTOREPOUPLOADBLOB_H

#include "atprotocol/accessatprotocol.h"
#include "tools/imagecompressor.h"

#include <QSize>
#include <QThread>

namespace AtProtocolInterface {

class ComAtprotoRepoUploadBlob : public AccessAtProtocol
{
    Q_OBJECT

public:
    explicit ComAtprotoRepoUploadBlob(QObject *parent = nullptr);
    ~ComAtprotoRepoUploadBlob();

    void uploadBlob(const QString &path);

    QString cid() const;
    QString mimeType() const;
    int size() const;
    QSize aspectRatio() const;

public slots:
    void compressed(const QString &path, const QSize &aspect_ratio);

signals:
    void compress(const QString &path);

private:
    virtual bool parseJson(bool success, const QString reply_json);

    ImageCompressor m_compressor;
    QThread m_thread;

    QString m_cid;
    QString m_mimeType;
    int m_size;
    QSize m_aspectRatio;
};

}

#endif // COMATPROTOREPOUPLOADBLOB_H
