#include "translator.h"
#include "encryption.h"

#include <QLocale>
#include <QSettings>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>

Translator::Translator(QObject *parent) : QObject { parent }
{
    qDebug().noquote() << this << "Translator()";
}

Translator::~Translator()
{
    qDebug().noquote() << this << "~Translator()";
}

Translator *Translator::getInstance()
{
    static Translator instance;
    return &instance;
}

void Translator::translate(const QString &cid, const QString &text)
{
    QNetworkRequest request((QUrl(apiUrl())));
    request.setRawHeader(QByteArray("Authorization"),
                         QByteArray("DeepL-Auth-Key ") + apiKey().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("text", text);
    params.addQueryItem("target_lang", targetLanguage());

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << "Translator reply" << reply->error() << reply->url();
        QString json = QString::fromUtf8(reply->readAll());
        QString text;

        if (reply->error() != QNetworkReply::NoError) {
            qCritical() << json;
        } else {
            QJsonDocument json_doc = QJsonDocument::fromJson(json.toUtf8());
            auto json_translations = json_doc.object().value("translations");
            if (json_translations.isArray()) {
                for (const auto &v : json_translations.toArray()) {
                    if (v.toObject().contains("text")) {
                        text += v.toObject().value("text").toString();
                    }
                }
            }
        }

        m_translations[cid] = text;
        emit finished(cid, text);
        reply->deleteLater();
        manager->deleteLater();
    });
    manager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());

    m_translations[cid] = "Now translating ...";
}

bool Translator::validSettings()
{
    Encryption encryption;
    QSettings settings;
    setApiUrl(settings.value("translateApiUrl").toString());
    setApiKey(encryption.decrypt(settings.value("translateApiKey").toString()));
    setTargetLanguage(settings.value("translateTargetLanguage").toString());

    return !(apiKey().isEmpty() || apiUrl().isEmpty() || targetLanguage().isEmpty());
}

QString Translator::getTranslation(const QString &cid) const
{
    return m_translations.value(cid, QString());
}

const QString Translator::apiUrl() const
{
    return m_apiUrl;
}

void Translator::setApiUrl(const QString &newApiUrl)
{
    if (m_apiUrl == newApiUrl)
        return;
    m_apiUrl = newApiUrl;
    emit apiUrlChanged();
}

const QString Translator::apiKey() const
{
    return m_apiKey;
}

void Translator::setApiKey(const QString &newApiKey)
{
    if (m_apiKey == newApiKey)
        return;
    m_apiKey = newApiKey;
    emit apiKeyChanged();
}

const QString Translator::targetLanguage() const
{
    return m_targetLanguage;
}

const QString Translator::targetLanguageBcp47() const
{
    QLocale l(m_targetLanguage);
    return l.bcp47Name();
}

void Translator::setTargetLanguage(const QString &newTargetLanguage)
{
    if (m_targetLanguage == newTargetLanguage)
        return;
    m_targetLanguage = newTargetLanguage;
    emit targetLanguageChanged();
}
