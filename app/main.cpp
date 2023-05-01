#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <qtquick/createsession.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<CreateSession>("tech.relog.hagoromo.createsession", 1, 0, "CreateSession");

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
