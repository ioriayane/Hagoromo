#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuickControls2/QQuickStyle>

#include "qtquick/createsession.h"
#include "qtquick/createrecord.h"
#include "qtquick/accountlistmodel.h"
#include "qtquick/columnlistmodel.h"
#include "qtquick/notificationlistmodel.h"
#include "qtquick/timelinelistmodel.h"
#include "qtquick/thumbnailprovider.h"

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

    qmlRegisterType<CreateSession>("tech.relog.hagoromo.createsession", 1, 0, "CreateSession");
    qmlRegisterType<CreateRecord>("tech.relog.hagoromo.createrecord", 1, 0, "CreateRecord");
    qmlRegisterType<AccountListModel>("tech.relog.hagoromo.accountlistmodel", 1, 0,
                                      "AccountListModel");
    qmlRegisterType<ColumnListModel>("tech.relog.hagoromo.columnlistmodel", 1, 0,
                                     "ColumnListModel");
    qmlRegisterType<NotificationListModel>("tech.relog.hagoromo.notificationlistmodel", 1, 0,
                                           "NotificationListModel");
    qmlRegisterType<TimelineListModel>("tech.relog.hagoromo.timelinelistmodel", 1, 0,
                                       "TimelineListModel");

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
