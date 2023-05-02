#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <qtquick/createsession.h>
#include <qtquick/accountlistmodel.h>

int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    app.setOrganizationName(QStringLiteral("relog"));
    app.setOrganizationDomain(QStringLiteral("hagoromo.relog.tech"));
    app.setApplicationName(QStringLiteral("Hagoromo"));

    qmlRegisterType<CreateSession>("tech.relog.hagoromo.createsession", 1, 0, "CreateSession");
    qmlRegisterType<AccountListModel>("tech.relog.hagoromo.accountlistmodel", 1, 0,
                                      "AccountListModel");

    QQmlApplicationEngine engine;
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
