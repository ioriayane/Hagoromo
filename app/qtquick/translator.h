#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QNetworkAccessManager>
#include <QObject>

class Translator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString apiUrl READ apiUrl WRITE setApiUrl NOTIFY apiUrlChanged)
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)
    Q_PROPERTY(QString targetLanguage READ targetLanguage WRITE setTargetLanguage NOTIFY
                       targetLanguageChanged)

public:
    explicit Translator(QObject *parent = nullptr);

    Q_INVOKABLE void translate(const QString &text);
    bool validSettings();

    const QString apiUrl() const;
    void setApiUrl(const QString &newApiUrl);
    const QString apiKey() const;
    void setApiKey(const QString &newApiKey);
    const QString targetLanguage() const;
    void setTargetLanguage(const QString &newTargetLanguage);

signals:
    void finished(const QString text);
    void apiUrlChanged();
    void apiKeyChanged();

    void targetLanguageChanged();

private:
    QNetworkAccessManager m_manager;

    QString m_apiUrl;
    QString m_apiKey;
    QString m_targetLanguage;
};

#endif // TRANSLATOR_H
