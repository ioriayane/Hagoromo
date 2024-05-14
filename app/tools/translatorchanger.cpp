#include "translatorchanger.h"

#include <QDebug>
#include <QQuickWindow>
#include <QSettings>

TranslatorChanger::TranslatorChanger(QObject *parent)
    : QObject { parent }, m_app(nullptr), m_engine(nullptr)
{
    m_currentLang = "en";
}

void TranslatorChanger::initialize(QCoreApplication *app, QQmlApplicationEngine *engine)
{
    m_app = app;
    m_engine = engine;

    const QStringList langs = QStringList() << "ja";
    QString dir = QString("%1/translations").arg(QCoreApplication::applicationDirPath());

    for (const auto &lang : langs) {
        QTranslator *t;
        t = new QTranslator(this);
        if (t->load(QString("app_%1").arg(lang), dir)) {
            m_translatorApp[lang] = t;
        } else {
            t->deleteLater();
        }
        t = new QTranslator(this);
        if (t->load(QString("qt_%1").arg(lang), dir)) {
            m_translatorSys[lang] = t;
        } else {
            t->deleteLater();
        }
    }
    qDebug() << "Loaded languages" << m_translatorApp.keys();
}

void TranslatorChanger::connect()
{
    if (m_engine == nullptr || m_engine->rootObjects().isEmpty())
        return;

    // QtObject {
    //     id: translatorChanger
    //     objectName: "translatorChanger"
    //     signal triggered(string lang)
    // }

    QQuickWindow *root = qobject_cast<QQuickWindow *>(m_engine->rootObjects().constFirst());
    if (!root)
        return;
    QObject *obj = root->findChild<QObject *>("translatorChanger");
    qDebug() << obj << obj->metaObject()->indexOfSignal("triggered(QString)");
    if (!obj)
        return;
    QObject::connect(obj, SIGNAL(triggered(QString)), this, SLOT(change(const QString &)));

    // change(const QString &)
}

void TranslatorChanger::setBySavedSetting()
{
    QSettings settings;
    QString lang = settings.value("language").toString();
    if (!m_translatorApp.contains(lang) && lang != "en") {
        lang = QLocale::system().bcp47Name();
        settings.setValue("language", lang);
    }
    change(lang);
}

void TranslatorChanger::change(const QString &lang)
{
    qDebug() << "Change language" << lang;
    if (m_app == nullptr || m_engine == nullptr || lang.isEmpty())
        return;
    if (m_currentLang == lang)
        return;

    if (m_currentLang != "en") {
        qDebug() << "Uninstall translator =" << m_currentLang;
        if (m_translatorApp.contains(m_currentLang)) {
            m_app->removeTranslator(m_translatorApp[m_currentLang]);
        }
        if (m_translatorSys.contains(m_currentLang)) {
            m_app->removeTranslator(m_translatorSys[m_currentLang]);
        }
    }
    if (m_translatorApp.contains(lang)) {
        qDebug() << "Install translator =" << lang;
        if (m_translatorApp.contains(lang)) {
            m_app->installTranslator(m_translatorApp[lang]);
        }
        if (m_translatorSys.contains(lang)) {
            m_app->installTranslator(m_translatorSys[lang]);
        }
    }
    m_currentLang = lang;

    m_engine->retranslate();
}
