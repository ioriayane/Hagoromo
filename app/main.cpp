#include <QGuiApplication>
#include <QQmlApplicationEngine>
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
    app.setApplicationVersion(QStringLiteral("0.2.0"));

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
    qmlRegisterType<Encryption>("tech.relog.hagoromo.encryption", 1, 0, "Encryption");
    qmlRegisterType<UserProfile>("tech.relog.hagoromo.userprofile", 1, 0, "UserProfile");
    qmlRegisterType<SystemTool>("tech.relog.hagoromo.systemtool", 1, 0, "SystemTool");

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
