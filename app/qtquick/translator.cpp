#include "translator.h"

#include <QSettings>
#include <QDebug>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>

Translator::Translator(QObject *parent) : QObject { parent }
{

    connect(&m_manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << "Translator reply" << reply->error() << reply->url();
        QString json = QString::fromUtf8(reply->readAll());
        QString text;

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << json;
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

        emit finished(text);
    });
}

void Translator::translate(const QString &text)
{
    QSettings settings;
    setApiUrl(settings.value("translateApiUrl").toString());
    setApiKey(settings.value("translateApiKey").toString());
    setTargetLanguage(settings.value("translateTargetLanguage").toString());

    QNetworkRequest request((QUrl(apiUrl())));
    request.setRawHeader(QByteArray("Authorization"),
                         QByteArray("DeepL-Auth-Key ") + apiKey().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("text", text);
    params.addQueryItem("target_lang", targetLanguage());

    m_manager.post(request, params.toString(QUrl::FullyEncoded).toUtf8());
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

void Translator::setTargetLanguage(const QString &newTargetLanguage)
{
    if (m_targetLanguage == newTargetLanguage)
        return;
    m_targetLanguage = newTargetLanguage;
    emit targetLanguageChanged();
}
