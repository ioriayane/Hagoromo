#ifndef CARDECODER_H
#define CARDECODER_H

#include <QByteArray>
#include <QStringList>

class CarDecoder
{
public:
    CarDecoder();

    bool setContent(const QByteArray &content);
    bool eof() const;
    bool next();
    QString cid() const;

private:
    QByteArray m_content;
    int m_offset;
    QString m_cid;
    QByteArray m_block;
    int m_lebSize;
    int m_dataSize;

    bool decodeData();
    QString decodeCid(const QByteArray &data, int &offset) const;
    bool decodeCbor(const QByteArray &block);
};

#endif // CARDECODER_H
