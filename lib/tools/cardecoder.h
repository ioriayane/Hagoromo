#ifndef CARDECODER_H
#define CARDECODER_H

#include <QByteArray>
#include <QCborValue>
#include <QJsonObject>
#include <QStringList>
#include <QHash>

class CarDecoder
{
public:
    CarDecoder();

    bool setContent(const QByteArray &content);
    bool eof() const;
    bool next();
    // 現在のDataブロックの情報
    QString cid() const;
    QString type() const;
    const QJsonObject &json() const;
    // すべて読み終わるまで確定しない
    QString did() const;
    QString uri(const QString &cid) const;

private:
    QByteArray m_content;
    int m_offset;
    QString m_cid;
    QString m_type;
    QByteArray m_block;
    QJsonObject m_json;
    QString m_did;
    QHash<QString, QString> m_cid2uri; // QHash<cid, uri>
    int m_lebSize;
    int m_dataSize;

    bool decodeData();
    QString decodeCid(const QByteArray &data, int &offset) const;
    bool decodeCbor(const QByteArray &block);
    bool decodeCborObject(const QCborValue &value, QJsonObject &parent);
    QVariant decodeCborValue(const QCborValue &value);
    void decodeCarAddress(const QJsonObject &json);
};

#endif // CARDECODER_H
