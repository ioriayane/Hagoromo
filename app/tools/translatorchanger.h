#ifndef TRANSLATORCHANGER_H
#define TRANSLATORCHANGER_H

#include <QCoreApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QHash>
#include <QTranslator>

class TranslatorChanger : public QObject
{
    Q_OBJECT
public:
    explicit TranslatorChanger(QObject *parent = nullptr);

    void initialize(QCoreApplication *app, QQmlApplicationEngine *engine);
    void connect();
    void setBySavedSetting();
signals:

public slots:
    void change(const QString &lang);

private:
    QCoreApplication *m_app;
    QQmlApplicationEngine *m_engine;
    QHash<QString, QTranslator *> m_translatorApp;
    QHash<QString, QTranslator *> m_translatorSys;
    QString m_currentLang;
};

#endif // TRANSLATORCHANGER_H
