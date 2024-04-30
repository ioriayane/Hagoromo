#include <QtTest>
#include <QCoreApplication>

#include "tools/base32.h"
#include "tools/leb128.h"
#include "tools/cardecoder.h"

class tools_test : public QObject
{
    Q_OBJECT

public:
    tools_test();
    ~tools_test();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_base32();
    void test_Leb128();
    void test_CarDecoder();
};

tools_test::tools_test() { }

tools_test::~tools_test() { }

void tools_test::initTestCase() { }

void tools_test::cleanupTestCase() { }

void tools_test::test_base32()
{
    {
        uint8_t buf[] = { 0x01, 0x55, 0x12, 0x20, 0xbc, 0xce, 0x40, 0xfd, 0x66, 0x2a, 0xac, 0x46,
                          0x0a, 0x89, 0xf5, 0xb7, 0xce, 0x36, 0xb4, 0x9e, 0xf7, 0xdf, 0xd8, 0x07,
                          0xd8, 0xe6, 0x05, 0x47, 0x02, 0x1b, 0x46, 0x29, 0x70, 0xd7, 0x6a, 0x64 };
        QString actual =
                Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)));
        QVERIFY2(actual == "afkreif4zzap2zrkvrdavcpvw7hdnne667p5qb6y4ycuoaq3iyuxbv3kmq",
                 actual.toLocal8Bit());
        actual = Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                                true);
        QVERIFY2(actual == "afkreif4zzap2zrkvrdavcpvw7hdnne667p5qb6y4ycuoaq3iyuxbv3kmq======",
                 actual.toLocal8Bit());
    }
    {
        uint8_t buf[] = { 0x01, 0x71, 0x12, 0x20, 0xaf, 0x9c, 0xf5, 0x65, 0x15, 0x54, 0x9f, 0x63,
                          0x62, 0xf6, 0xa9, 0x53, 0x1f, 0xe4, 0xc0, 0x3f, 0xee, 0xba, 0xe5, 0x0c,
                          0xc3, 0xf6, 0x2b, 0x11, 0xbd, 0xbd, 0x68, 0x35, 0xcb, 0x06, 0x0b, 0xc6 };
        QString actual =
                Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)));
        QVERIFY2(actual == "afyreifptt2wkfkut5rwf5vjkmp6jqb7525okdgd6yvrdpn5na24wbqlyy",
                 actual.toLocal8Bit());
        actual = Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                                true);
        QVERIFY2(actual == "afyreifptt2wkfkut5rwf5vjkmp6jqb7525okdgd6yvrdpn5na24wbqlyy======",
                 actual.toLocal8Bit());
    }

    {
        uint8_t buf[] = { 0x00 };
        QString actual =
                Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)));
        QVERIFY2(actual == "aa", actual.toLocal8Bit());
        actual = Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                                true);
        QVERIFY2(actual == "aa======", actual.toLocal8Bit());
    }
    {
        QString actual = Base32::encode(QByteArray());
        QVERIFY2(actual == QString(), actual.toLocal8Bit());
        actual = Base32::encode(QByteArray(), true);
        QVERIFY2(actual == QString(), actual.toLocal8Bit());
    }
}

void tools_test::test_Leb128()
{
    quint8 buf[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    int offset;
    int v;

    memset(buf, 0, sizeof(buf));
    buf[0] = 0x20;
    offset = 0;
    v = Leb128::decode_u(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                         offset);
    QVERIFY2(v == 32, QString::number(v).toLocal8Bit());
    QVERIFY2(offset == 1, QString::number(v).toLocal8Bit());

    memset(buf, 0, sizeof(buf));
    buf[0] = 0xff;
    buf[1] = 0xaf;
    offset = 0;
    v = Leb128::decode_u(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                         offset);
    QVERIFY2(v == 6143, QString::number(v).toLocal8Bit());
    QVERIFY2(offset == 3, QString::number(offset).toLocal8Bit());

    memset(buf, 0, sizeof(buf));
    buf[0] = 0x3a;
    buf[1] = 0xa2;
    buf[2] = 0x65;
    offset = 0;
    v = Leb128::decode_u(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                         offset);
    QVERIFY2(v == 58, QString::number(v).toLocal8Bit());
    QVERIFY2(offset == 1, QString::number(offset).toLocal8Bit());

    memset(buf, 0, sizeof(buf));
    buf[0] = 0xd7;
    buf[1] = 0x09;
    buf[2] = 0x01;
    offset = 0;
    v = Leb128::decode_u(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                         offset);
    QVERIFY2(v == 1239, QString::number(v).toLocal8Bit());
    QVERIFY2(offset == 2, QString::number(offset).toLocal8Bit());
}

void tools_test::test_CarDecoder()
{
    CarDecoder decoder;

    QFile repo(":/response/xrpc/com.atproto.sync.getRepo");
    QVERIFY(repo.open(QFile::ReadOnly));
    QVERIFY(decoder.setContent(repo.readAll()));

    int i = 0;
    while (!decoder.eof()) {
        if (i == 0) {
            QVERIFY2(decoder.cid() == "bafyreihz3ltonllans77xjbngp2qilac2ui6k5pqx4i4u6p4k3pnpr53bu",
                     decoder.cid().toLocal8Bit());
        } else if (i == 22) {
            QVERIFY2(decoder.cid() == "bafyreifptt2wkfkut5rwf5vjkmp6jqb7525okdgd6yvrdpn5na24wbqlyy",
                     decoder.cid().toLocal8Bit());
        } else if (i == 713) {
            QVERIFY2(decoder.cid() == "bafyreia2udub3oplahxbj2akwytpiu6bopoxkox2locgzns6cieuebcqi4",
                     decoder.cid().toLocal8Bit());
        }

        i++;
        decoder.next();
    }

    repo.close();
}

QTEST_MAIN(tools_test)

#include "tst_tools_test.moc"
