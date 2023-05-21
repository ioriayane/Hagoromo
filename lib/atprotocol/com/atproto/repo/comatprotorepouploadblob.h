#ifndef COMATPROTOREPOUPLOADBLOB_H
#define COMATPROTOREPOUPLOADBLOB_H

#include "atprotocol/accessatprotocol.h"
#include "tools/imagecompressor.h"

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

public slots:
    void compressed(const QString &path);

signals:
    void compress(const QString &path);

private:
    virtual void parseJson(const QString reply_json);

    ImageCompressor m_compressor;
    QThread m_thread;

    QString m_cid;
    QString m_mimeType;
    int m_size;
};

}

#endif // COMATPROTOREPOUPLOADBLOB_H
