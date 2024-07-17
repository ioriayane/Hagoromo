#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QHash>
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
    ~Translator();

    static Translator *getInstance();

    Q_INVOKABLE void translate(const QString &cid, const QString &text);
    bool validSettings();
    QString getTranslation(const QString &cid) const;

    const QString apiUrl() const;
    void setApiUrl(const QString &newApiUrl);
    const QString apiKey() const;
    void setApiKey(const QString &newApiKey);
    const QString targetLanguage() const;
    const QString targetLanguageBcp47() const;
    void setTargetLanguage(const QString &newTargetLanguage);

signals:
    void finished(const QString &cid, const QString text);
    void apiUrlChanged();
    void apiKeyChanged();

    void targetLanguageChanged();

private:
    QHash<QString, QString> m_translations; // QHash<cid, translation>

    QString m_apiUrl;
    QString m_apiKey;
    QString m_targetLanguage;
};

#endif // TRANSLATOR_H
