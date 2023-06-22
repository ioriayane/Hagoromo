#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QTranslator>
#include <QtQuickControls2/QQuickStyle>

#include "qtquick/createsession.h"
#include "qtquick/recordoperator.h"
#include "qtquick/accountlistmodel.h"
#include "qtquick/columnlistmodel.h"
#include "qtquick/notificationlistmodel.h"
#include "qtquick/postthreadlistmodel.h"
#include "qtquick/timelinelistmodel.h"
#include "qtquick/authorfeedlistmodel.h"
#include "qtquick/anyfeedlistmodel.h"
#include "qtquick/followslistmodel.h"
#include "qtquick/followerslistmodel.h"
#include "qtquick/searchpostlistmodel.h"
#include "qtquick/searchprofilelistmodel.h"
#include "qtquick/feedtypelistmodel.h"
#include "qtquick/customfeedlistmodel.h"
#include "qtquick/feedgeneratorlistmodel.h"
#include "qtquick/thumbnailprovider.h"
#include "qtquick/encryption.h"
#include "qtquick/userprofile.h"
#include "qtquick/systemtool.h"

int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Material");

    app.setOrganizationName(QStringLiteral("relog"));
    app.setOrganizationDomain(QStringLiteral("hagoromo.relog.tech"));
    app.setApplicationName(QStringLiteral("Hagoromo"));
    app.setApplicationVersion(QStringLiteral("0.3.0"));

    qmlRegisterType<CreateSession>("tech.relog.hagoromo.createsession", 1, 0, "CreateSession");
    qmlRegisterType<RecordOperator>("tech.relog.hagoromo.recordoperator", 1, 0, "RecordOperator");
    qmlRegisterType<AccountListModel>("tech.relog.hagoromo.accountlistmodel", 1, 0,
                                      "AccountListModel");
    qmlRegisterType<ColumnListModel>("tech.relog.hagoromo.columnlistmodel", 1, 0,
                                     "ColumnListModel");
    qmlRegisterType<NotificationListModel>("tech.relog.hagoromo.notificationlistmodel", 1, 0,
                                           "NotificationListModel");
    qmlRegisterType<PostThreadListModel>("tech.relog.hagoromo.postthreadlistmodel", 1, 0,
                                         "PostThreadListModel");
    qmlRegisterType<TimelineListModel>("tech.relog.hagoromo.timelinelistmodel", 1, 0,
                                       "TimelineListModel");
    qmlRegisterType<AuthorFeedListModel>("tech.relog.hagoromo.authorfeedlistmodel", 1, 0,
                                         "AuthorFeedListModel");
    qmlRegisterType<AnyFeedListModel>("tech.relog.hagoromo.anyfeedlistmodel", 1, 0,
                                      "AnyFeedListModel");
    qmlRegisterType<FollowsListModel>("tech.relog.hagoromo.followslistmodel", 1, 0,
                                      "FollowsListModel");
    qmlRegisterType<FollowersListModel>("tech.relog.hagoromo.followerslistmodel", 1, 0,
                                        "FollowersListModel");
    qmlRegisterType<SearchPostListModel>("tech.relog.hagoromo.searchpostlistmodel", 1, 0,
                                         "SearchPostListModel");
    qmlRegisterType<SearchProfileListModel>("tech.relog.hagoromo.searchprofilelistmodel", 1, 0,
                                            "SearchProfileListModel");
    qmlRegisterType<FeedTypeListModel>("tech.relog.hagoromo.feedtypelistmodel", 1, 0,
                                       "FeedTypeListModel");
    qmlRegisterType<CustomFeedListModel>("tech.relog.hagoromo.customfeedlistmodel", 1, 0,
                                         "CustomFeedListModel");
    qmlRegisterType<FeedGeneratorListModel>("tech.relog.hagoromo.feedgeneratorlistmodel", 1, 0,
                                            "FeedGeneratorListModel");
    qmlRegisterType<Encryption>("tech.relog.hagoromo.encryption", 1, 0, "Encryption");
    qmlRegisterType<UserProfile>("tech.relog.hagoromo.userprofile", 1, 0, "UserProfile");
    qmlRegisterType<SystemTool>("tech.relog.hagoromo.systemtool", 1, 0, "SystemTool");

    QString dir = QString("%1/translations").arg(QCoreApplication::applicationDirPath());
    // 翻訳データ登録
    QTranslator translator;
    if (translator.load(QString("qt_%1").arg(QLocale::system().name()), dir)) {
        app.installTranslator(&translator);
    }
    // Qt標準機能の日本語化
    QTranslator translator2;
    if (translator2.load(QString("qt_ja"), dir)) {
        app.installTranslator(&translator2);
    }

#ifdef QT_NO_DEBUG
    QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "*.info=false\n"
                                     "*.warning=true\n"
                                     "*.critical=true");
#endif

    QQmlApplicationEngine engine;

    engine.addImageProvider(QStringLiteral("thumbnail"), new ThumbnailProvider);

    const QUrl url("qrc:/Hagoromo/qml/main.qml");
    QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreated, &app,
            [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl)
                    QCoreApplication::exit(-1);
            },
            Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
