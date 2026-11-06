#include <QtTest>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "tools/base32.h"
#include "tools/leb128.h"
#include "tools/cardecoder.h"
#include "tools/tid.h"
#include "tools/supportprovider.h"
#include "tools/encryption.h"
#include "tools/mochottlongpost.h"
#include "atprotocol/lexicons_func.h"

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
    void test_base32_s();
    void test_tid();
    void test_SupportProvider();
    void test_Leb128();
    void test_CarDecoder();
    void test_Encryption();
    void test_MochottLongPost_parseReference();
    void test_MochottLongPost_parseOpening();
    void test_MochottLongPost_parseArticle();
    void test_MochottLongPost_parseArticle_invalid();
};

tools_test::tools_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));
}

tools_test::~tools_test() { }

void tools_test::initTestCase() { }

void tools_test::cleanupTestCase() { }

void tools_test::test_Encryption()
{
    Encryption encryption;
    const QStringList plains = QStringList()
            << QString() << "a"
            << "password" << QString("0123456789abcdef") // ブロック長ちょうど
            << QString::fromUtf8("日本語のパスワード")
            << "-----BEGIN PRIVATE KEY-----\nMIGHAgEAMBMGByqGSM49\n-----END PRIVATE KEY-----\n"
            << QString(1000, 'x');

    for (const auto &plain : plains) {
        const QString encrypted = encryption.encrypt(plain);
        QVERIFY(encrypted.startsWith("v2:"));
        QVERIFY(!encrypted.contains(plain) || plain.length() < 2);
        QCOMPARE(encryption.decrypt(encrypted), plain);

        // 同じ平文でも毎回違う暗号文になる(IV/nonceが固定でない)
        const QString encrypted2 = encryption.encrypt(plain);
        QVERIFY(encrypted != encrypted2);
        QCOMPARE(encryption.decrypt(encrypted2), plain);

        // 別のインスタンスでも復号できる
        Encryption other;
        QCOMPARE(other.decrypt(encrypted), plain);

        // 従来形式(IV固定のCBC)で保存したものも読める
        const QString legacy = encryption.encryptLegacy(plain);
        QVERIFY(!legacy.startsWith("v2:"));
        QCOMPARE(encryption.decrypt(legacy), plain);
    }

    // 改ざんされたものは復号しない
    {
        const QString encrypted = encryption.encrypt("password");
        QByteArray raw = QByteArray::fromBase64(encrypted.mid(3).toUtf8());
        for (int i = 0; i < raw.size(); i++) {
            QByteArray tampered = raw;
            tampered[i] = static_cast<char>(tampered.at(i) ^ 0x01);
            QCOMPARE(encryption.decrypt("v2:" + QString::fromUtf8(tampered.toBase64())), QString());
        }
        // 短すぎる・壊れている
        QCOMPARE(encryption.decrypt("v2:"), QString());
        QCOMPARE(encryption.decrypt("v2:AAAA"), QString());
        QCOMPARE(encryption.decrypt("v2:!!!!"), QString());
    }
    QCOMPARE(encryption.decrypt(QString()), QString());
}

void tools_test::test_base32()
{
    {
        uint8_t buf[] = { 0x01, 0x55, 0x12, 0x20, 0xbc, 0xce, 0x40, 0xfd, 0x66, 0x2a, 0xac, 0x46,
                          0x0a, 0x89, 0xf5, 0xb7, 0xce, 0x36, 0xb4, 0x9e, 0xf7, 0xdf, 0xd8, 0x07,
                          0xd8, 0xe6, 0x05, 0x47, 0x02, 0x1b, 0x46, 0x29, 0x70, 0xd7, 0x6a, 0x64 };
        QString actual =
                Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)));
        QCOMPARE(actual, "afkreif4zzap2zrkvrdavcpvw7hdnne667p5qb6y4ycuoaq3iyuxbv3kmq");
        actual = Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                                true);
        QCOMPARE(actual, "afkreif4zzap2zrkvrdavcpvw7hdnne667p5qb6y4ycuoaq3iyuxbv3kmq======");
    }
    {
        uint8_t buf[] = { 0x01, 0x71, 0x12, 0x20, 0xaf, 0x9c, 0xf5, 0x65, 0x15, 0x54, 0x9f, 0x63,
                          0x62, 0xf6, 0xa9, 0x53, 0x1f, 0xe4, 0xc0, 0x3f, 0xee, 0xba, 0xe5, 0x0c,
                          0xc3, 0xf6, 0x2b, 0x11, 0xbd, 0xbd, 0x68, 0x35, 0xcb, 0x06, 0x0b, 0xc6 };
        QString actual =
                Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)));
        QCOMPARE(actual, "afyreifptt2wkfkut5rwf5vjkmp6jqb7525okdgd6yvrdpn5na24wbqlyy");
        actual = Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                                true);
        QCOMPARE(actual, "afyreifptt2wkfkut5rwf5vjkmp6jqb7525okdgd6yvrdpn5na24wbqlyy======");
    }

    {
        uint8_t buf[] = { 0x00 };
        QString actual =
                Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)));
        QCOMPARE(actual, "aa");
        actual = Base32::encode(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                                true);
        QCOMPARE(actual, "aa======");
    }
    {
        QString actual = Base32::encode(QByteArray());
        QCOMPARE(actual, QString());
        actual = Base32::encode(QByteArray(), true);
        QCOMPARE(actual, QString());
    }
}

void tools_test::test_base32_s()
{
    QCOMPARE(Base32::encode_s(0), "2222222222222");
}

void tools_test::test_tid()
{
    QString prev = "";
    QString current;

    for (int i = 0; i < 1000; i++) {
        current = Tid::next();
        QCOMPARE_LT(prev, current);
        prev = current;
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
    QCOMPARE(v, 32);
    QCOMPARE(offset, 1);

    memset(buf, 0, sizeof(buf));
    buf[0] = 0xff;
    buf[1] = 0xaf;
    offset = 0;
    v = Leb128::decode_u(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                         offset);
    QCOMPARE(v, 6143);
    QCOMPARE(offset, 3);

    memset(buf, 0, sizeof(buf));
    buf[0] = 0x3a;
    buf[1] = 0xa2;
    buf[2] = 0x65;
    offset = 0;
    v = Leb128::decode_u(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                         offset);
    QCOMPARE(v, 58);
    QCOMPARE(offset, 1);

    memset(buf, 0, sizeof(buf));
    buf[0] = 0xd7;
    buf[1] = 0x09;
    buf[2] = 0x01;
    offset = 0;
    v = Leb128::decode_u(QByteArray::fromRawData(reinterpret_cast<char *>(buf), sizeof(buf)),
                         offset);
    QCOMPARE(v, 1239);
    QCOMPARE(offset, 2);
}

void tools_test::test_CarDecoder()
{
    CarDecoder decoder;

    QFile repo(":/response/xrpc/com.atproto.sync.getRepo");
    QVERIFY(repo.open(QFile::ReadOnly));
    QVERIFY(decoder.setContent(repo.readAll()));

    QJsonObject except_json;
    QFile file(":/data/car/records.json");
    if (file.open(QFile::ReadOnly)) {
        except_json = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    }
    QHash<QString, QJsonObject> except_hash;
    for (const auto &record : except_json.value("records").toArray()) {
        QString cid = record.toObject().value("cid").toString();
        if (!cid.isEmpty()) {
            except_hash[cid] = record.toObject();
            // qDebug() << cid << except_hash[cid].value("$type").toString();
        }
    }

    QCOMPARE(decoder.cids().length(), 718);

    for (const auto &cid : decoder.cids()) {
        if (decoder.type(cid) != "$car_address") {
            bool check = (except_hash[cid].value("value").toObject() == decoder.json(cid));
            if (!check) {
                qDebug().nospace().noquote() << "cid:" << cid;
                qDebug().nospace().noquote()
                        << "except:"
                        << QJsonDocument(except_hash[cid].value("value").toObject()).toJson();
                qDebug().nospace().noquote()
                        << "actual:" << QJsonDocument(decoder.json(cid)).toJson();
            }
            QVERIFY(check);

            QCOMPARE(except_hash[cid].value("uri").toString(), decoder.uri(cid));
        } else {
            // qDebug().nospace().noquote() << "cid:" << cid;
            // qDebug().nospace().noquote() << "actual:" <<
            // QJsonDocument(decoder.json(cid)).toJson();
        }
    }

    QVERIFY(decoder.headerJson().value("roots").isArray());
    QCOMPARE(decoder.headerJson().value("roots").toArray().count(), 1);
    QVERIFY(decoder.headerJson().value("roots").toArray().at(0).toObject().value("$link").toString()
            == "bafyreia5is2dgfsipq3qdj4bya7kb7cv32534sfwutiffh6kwgle4p26je");

    // qDebug().nospace().noquote() << "header:" << QJsonDocument(decoder.headerJson()).toJson();

    repo.close();
}

void tools_test::test_SupportProvider()
{
    // アカウントのページ（パス or サブドメイン）
    QCOMPARE(SupportProvider::name("https://ko-fi.com/name"), "Ko-fi");
    QCOMPARE(SupportProvider::name("https://www.patreon.com/name/"), "Patreon");
    QCOMPARE(SupportProvider::name("https://name.itch.io"), "itch.io");
    QCOMPARE(SupportProvider::name("https://name.booth.pm/items/123"), "BOOTH");
    QCOMPARE(SupportProvider::name("http://KO-FI.COM/Name"), "Ko-fi");
    QCOMPARE(SupportProvider::name("https://buy.stripe.com/abc"), "Stripe");
    QCOMPARE(SupportProvider::name("https://paypal.me/name"), "PayPal");
    QCOMPARE(SupportProvider::name("https://www.paypal.com/paypalme/name"), "PayPal");
    QCOMPARE(SupportProvider::name("https://chuffed.org/project/name"), "Chuffed");
    QVERIFY(SupportProvider::isSupportProvider("https://fanbox.cc/@name"));
    QCOMPARE(SupportProvider::logo("https://ko-fi.com/name"), "KoFi");
    QCOMPARE(SupportProvider::logo("https://www.paypal.com/paypalme/name"), "PayPal");
    QCOMPARE(SupportProvider::logo("https://fanbox.cc/@name"), "PixivFanbox");
    QCOMPARE(SupportProvider::logo("https://ko-fi.com"), QString());

    // トップページは対象外
    QCOMPARE(SupportProvider::name("https://ko-fi.com"), QString());
    QCOMPARE(SupportProvider::name("https://ko-fi.com/"), QString());
    QCOMPARE(SupportProvider::name("https://www.patreon.com/"), QString());
    QCOMPARE(SupportProvider::name("https://buy.stripe.com/"), QString());
    // パスの指定があるものはそのパス配下のみ
    QCOMPARE(SupportProvider::name("https://www.paypal.com/signin"), QString());
    QCOMPARE(SupportProvider::name("https://www.paypal.com/paypalme/"), QString());
    QCOMPARE(SupportProvider::name("https://chuffed.org/about"), QString());
    // 似たドメインは対象外
    QCOMPARE(SupportProvider::name("https://ko-fi.com.example.net/name"), QString());
    QCOMPARE(SupportProvider::name("https://notko-fi.com/name"), QString());
    QCOMPARE(SupportProvider::name("https://stripe.com/name"), QString());
    // Webのリンク以外
    QCOMPARE(SupportProvider::name("ftp://ko-fi.com/name"), QString());
    QCOMPARE(SupportProvider::name("ko-fi.com/name"), QString());
    QCOMPARE(SupportProvider::name(""), QString());
    QVERIFY(!SupportProvider::isSupportProvider("https://github.com/ioriayane/Hagoromo"));
}

namespace {

const QString MOCHOTT_DID = QStringLiteral("did:plc:author");
const QString MOCHOTT_URI = QStringLiteral("at://did:plc:author/site.mochott.article/3m4abc");
const QString MOCHOTT_CID = QStringLiteral("bafyreiarticle");

AtProtocolType::AppBskyFeedPost::Main makeMochottRecord(const QByteArray &json)
{
    AtProtocolType::AppBskyFeedPost::Main record;
    AtProtocolType::AppBskyFeedPost::copyMain(QJsonDocument::fromJson(json).object(), record);
    return record;
}

MochottLongPost::Article parseMochottArticle(const QByteArray &content,
                                             const AtProtocolType::AppBskyFeedPost::Main &record =
                                                     AtProtocolType::AppBskyFeedPost::Main())
{
    QJsonObject article;
    article.insert("uri", MOCHOTT_URI);
    article.insert("cid", MOCHOTT_CID);
    QJsonObject doc;
    doc.insert("type", "doc");
    doc.insert("content", QJsonDocument::fromJson(content).array());
    article.insert("contentJSON", doc);

    MochottLongPost::Reference reference;
    reference.uri = MOCHOTT_URI;
    reference.cid = MOCHOTT_CID;
    reference.rkey = QStringLiteral("3m4abc");
    return MochottLongPost::parseArticle(article, reference, record, MOCHOTT_DID);
}

}

void tools_test::test_MochottLongPost_parseReference()
{
    struct TestCase
    {
        QString uri;
        QString cid;
        QString rkey;
    };
    const QList<TestCase> cases = QList<TestCase>()
            << TestCase { MOCHOTT_URI, MOCHOTT_CID, "3m4abc" } // CIDが無い
            << TestCase { MOCHOTT_URI, QString(), QString() } // URIが無い
            << TestCase { QString(), MOCHOTT_CID, QString() } // 投稿者と違う
            << TestCase { "at://did:plc:other/site.mochott.article/3m4abc", MOCHOTT_CID, QString() }
            // コレクションが違う
            << TestCase { "at://did:plc:author/app.bsky.feed.post/3m4abc", MOCHOTT_CID, QString() }
            // rkeyに使えない文字
            << TestCase { "at://did:plc:author/site.mochott.article/3m4/abc", MOCHOTT_CID,
                          QString() };
    for (const auto &item : cases) {
        AtProtocolType::AppBskyFeedPost::Main record;
        record.site_mochott_longPost.uri = item.uri;
        record.site_mochott_longPost.cid = item.cid;
        const auto reference = MochottLongPost::parseReference(record, MOCHOTT_DID);
        QCOMPARE(reference.rkey, item.rkey);
        QCOMPARE(reference.isValid(), !item.rkey.isEmpty());
    }

    // 文字列以外は無視する
    const auto record =
            makeMochottRecord(R"({"text":"a","site.mochott.longPost":{"uri":1,"cid":{"a":"b"}}})");
    QVERIFY(!MochottLongPost::parseReference(record, MOCHOTT_DID).isValid());
}

void tools_test::test_MochottLongPost_parseOpening()
{
    // 🌈(4)本(3)文(3)…(3)=13 もっと見る(15)=28
    {
        const auto record = makeMochottRecord(u8R"({
            "text":"🌈本文…もっと見る",
            "facets":[
                {"index":{"byteStart":4,"byteEnd":7},"features":[{"$type":"app.bsky.richtext.facet#tag","tag":"t"}]},
                {"index":{"byteStart":13,"byteEnd":28},"features":[{"$type":"app.bsky.richtext.facet#link","uri":"https://mochott.site/alice/3m4abc"}]}
            ]})");
        const auto opening = MochottLongPost::parseOpening(record, "3m4abc");
        QCOMPARE(opening.text, QString::fromUtf8("🌈本文…"));
        QCOMPARE(opening.article_url, QString("https://mochott.site/alice/3m4abc"));
        QCOMPARE(opening.facets.count(), 1);
        QCOMPARE(opening.facets.first().features_Tag.first().tag, QString("t"));
    }
    struct TestCase
    {
        QByteArray facets;
    };
    const QList<TestCase> cases = QList<TestCase>()
            // 末尾ではない
            << TestCase { R"([{"index":{"byteStart":13,"byteEnd":27},"features":[{"$type":"app.bsky.richtext.facet#link","uri":"https://mochott.site/alice/3m4abc"}]}])" }
            // ドメインが違う
            << TestCase { R"([{"index":{"byteStart":13,"byteEnd":28},"features":[{"$type":"app.bsky.richtext.facet#link","uri":"https://example.com/alice/3m4abc"}]}])" }
            // rkeyが違う
            << TestCase { R"([{"index":{"byteStart":13,"byteEnd":28},"features":[{"$type":"app.bsky.richtext.facet#link","uri":"https://mochott.site/alice/3m4xyz"}]}])" }
            // リンクではない
            << TestCase {
                   R"([{"index":{"byteStart":13,"byteEnd":28},"features":[{"$type":"app.bsky.richtext.facet#tag","tag":"3m4abc"}]}])"
               };
    for (const auto &item : cases) {
        const auto record = makeMochottRecord(
                QByteArray(u8R"({"text":"🌈本文…もっと見る","facets":)") + item.facets + "}");
        const auto opening = MochottLongPost::parseOpening(record, "3m4abc");
        QCOMPARE(opening.text, record.text);
        QCOMPARE(opening.facets.count(), 1);
        QVERIFY(opening.article_url.isEmpty());
    }
}

void tools_test::test_MochottLongPost_parseArticle()
{
    {
        // 実際の応答と同じく末尾の空の段落は取り除き、途中の空の段落は残す
        const auto article = parseMochottArticle(u8R"JSON([
            {"type":"paragraph","content":[{"type":"text","text":"一<b>行目"},{"type":"hardBreak"},{"type":"text","text":"二行目"}]},
            {"type":"paragraph"},
            {"type":"paragraph","content":[
                {"type":"text","text":"link","marks":[{"type":"link","attrs":{"href":"https://example.com/?a=1&b=\"2\""}}]},
                {"type":"text","text":"bad","marks":[{"type":"link","attrs":{"href":"javascript:alert(1)"}}]},
                {"type":"text","text":"日本語","marks":[{"type":"link","attrs":{"href":"https://example.com/日本語"}}]}
            ]},
            {"type":"paragraph"},
            {"type":"paragraph"}
        ])JSON");
        QVERIFY(article.valid);
        QVERIFY(!article.edited);
        QCOMPARE(article.text,
                 QString::fromUtf8("一&lt;b&gt;行目<br/>二行目<br/><br/>"
                                   "<a href=\"https://example.com/?a=1&amp;b=&quot;2&quot;\">"
                                   "link</a>bad"
                                   "<a href=\"https://example.com/日本語\">日本語</a>"));
        QCOMPARE(article.text_plain, QString::fromUtf8("一<b>行目\n二行目\n\nlinkbad日本語"));
    }
    {
        // ポストのembedと重複するものは除き、残りを表示する
        const auto record = makeMochottRecord(R"({
            "text":"a",
            "embed":{
                "$type":"app.bsky.embed.recordWithMedia",
                "record":{"$type":"app.bsky.embed.record","record":{"uri":"at://did:plc:other/app.bsky.feed.post/3post","cid":"c"}},
                "media":{"$type":"app.bsky.embed.images","images":[{"alt":"","image":{"$type":"blob","ref":{"$link":"bafkreisame"},"mimeType":"image/jpeg","size":1}}]}
            }})");
        const auto article = parseMochottArticle(R"([
            {"type":"paragraph","content":[{"type":"text","text":"body"}]},
            {"type":"image","attrs":{"src":"/api/image/did:plc:author/bafkreisame"}},
            {"type":"image","attrs":{"src":"/api/image/did:plc:author/bafkreione","alt":"one","width":100,"height":200}},
            {"type":"image","attrs":{"src":"/api/image/did:plc:author/bafkreione"}},
            {"type":"image","attrs":{"src":"/api/image/did:plc:author/bafkreitwo","alt":"two"}},
            {"type":"image","attrs":{"src":"/api/image/did:plc:other/bafkreiother"}},
            {"type":"image","attrs":{"src":"https://example.com/a.png"}},
            {"type":"linkCard","attrs":{"url":"https://example.com/","title":"<T>"}},
            {"type":"linkCard","attrs":{"url":"file:///etc/passwd","title":"bad"}},
            {"type":"embed","attrs":{"src":"https://bsky.app/profile/did:plc:other/post/3post","service":"bluesky"}},
            {"type":"embed","attrs":{"src":"https://bsky.app/profile/did:plc:other/post/3rest","service":"bluesky"}},
            {"type":"embed","attrs":{"src":"https://example.com/post","service":"bluesky"}},
            {"type":"embed","attrs":{"src":"https://bsky.app/profile/did:plc:other/post/3svc","service":"other"}}
        ])",
                                                 record);
        QVERIFY(article.valid);
        QCOMPARE(article.images,
                 QStringList() << "https://mochott.site/api/image/did:plc:author/bafkreione"
                               << "https://mochott.site/api/image/did:plc:author/bafkreitwo");
        QCOMPARE(article.images_alt,
                 QStringList() << "one"
                               << "two");
        // サイズの分からない画像があれば使わない
        QVERIFY(article.images_ratio.isEmpty());
        QCOMPARE(article.text,
                 QString("body<br/><a href=\"https://example.com/\">&lt;T&gt;</a><br/>"
                         "<a href=\"https://bsky.app/profile/did:plc:other/post/3rest\">"
                         "https://bsky.app/profile/did:plc:other/post/3rest</a>"));
    }
}

void tools_test::test_MochottLongPost_parseArticle_invalid()
{
    const QList<QByteArray> contents = QList<QByteArray>()
            // 表にないノードやマーク
            << R"([{"type":"heading","content":[{"type":"text","text":"a"}]}])"
            << R"([{"type":"paragraph","content":[{"type":"mention","attrs":{}}]}])"
            << R"([{"type":"paragraph","content":[{"type":"text","text":"a","marks":[{"type":"bold"}]}]}])"
            // 表示するものがない
            << R"([])"
            << R"([{"type":"paragraph"}])";
    for (const auto &content : contents) {
        QVERIFY2(!parseMochottArticle(content).valid, content.constData());
    }

    // 要求と異なるURIの記事は使わない
    QJsonObject article;
    article.insert("uri", "at://did:plc:author/site.mochott.article/3m4xyz");
    article.insert("cid", MOCHOTT_CID);
    MochottLongPost::Reference reference;
    reference.uri = MOCHOTT_URI;
    reference.cid = MOCHOTT_CID;
    reference.rkey = QStringLiteral("3m4abc");
    QVERIFY(!MochottLongPost::parseArticle(article, reference,
                                           AtProtocolType::AppBskyFeedPost::Main(), MOCHOTT_DID)
                     .valid);
}

QTEST_MAIN(tools_test)

#include "tst_tools_test.moc"
