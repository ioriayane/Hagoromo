#include <QtTest>
#include <QCoreApplication>

#include "tools/base32.h"

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

QTEST_MAIN(tools_test)

#include "tst_tools_test.moc"
