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
    CarDecoder(bool forFirehose = false);

    bool setContent(const QByteArray &content);
    QStringList cids() const;
    QString type(const QString &cid) const;
    QJsonObject json(const QString &cid) const;
    QJsonObject headerJson() const;
    QString did() const;
    QString uri(const QString &cid) const;

    int decodeCbor(const QByteArray &block, const QString &cid);

private:
    QByteArray m_content;
    QString m_did;
    QStringList m_cids;
    QHash<QString, QString> m_cid2type;
    QHash<QString, QJsonObject> m_cid2Json;
    QHash<QString, QString> m_cid2uri; // QHash<cid, uri>
    bool m_forFirehose;

    int decodeData(int offset);
    QString decodeCid(const QByteArray &data, int &offset) const;
    bool decodeCborObject(const QCborValue &value, QJsonObject &parent);
    QVariant decodeCborValue(const QCborValue &value);
    void decodeCarAddress(const QJsonObject &json);
};

#endif // CARDECODER_H
