#include "appbskyactorputpreferencesex.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace AtProtocolInterface {

AppBskyActorPutPreferencesEx::AppBskyActorPutPreferencesEx(QObject *parent)
    : AppBskyActorPutPreferences { parent }
{
}

bool AppBskyActorPutPreferencesEx::parseJson(bool success, const QString reply_json)
{
    success = AppBskyActorPutPreferences::parseJson(success, reply_json);

    return success;
}

QJsonArray AppBskyActorPutPreferencesEx::updatePreferencesJson(const QString &src,
                                                               const QString &type,
                                                               const QJsonObject &part)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(src.toUtf8());
    QJsonObject root_object = json_doc.object();
    if (!root_object.contains("preferences"))
        return QJsonArray();

    QJsonValue preferences = root_object.value("preferences");
    QJsonArray dest_preferences;
    if (!preferences.isArray())
        return QJsonArray();

    bool replaced = false;
    for (int i = 0; i < preferences.toArray().count(); i++) {
        if (!preferences.toArray().at(i).isObject())
            continue;
        QJsonObject value = preferences.toArray().takeAt(i).toObject();
        if (value.value("$type") == type) {
            dest_preferences.append(part);
            replaced = true;
        } else {
            // その他のデータはそのまま引き継ぐ
            dest_preferences.append(value);
        }
    }

    if (!replaced) {
        dest_preferences.append(part);
    }

    return dest_preferences;
}

QJsonObject AppBskyActorPutPreferencesEx::makePostInteractionSettingsPref(
        const QString &thread_gate_type, const QStringList &thread_gate_options,
        const bool post_gate_quote_enabled) const
{
    QJsonObject part;
    part.insert("$type", "app.bsky.actor.defs#postInteractionSettingsPref");
    if (post_gate_quote_enabled) {
        QJsonArray post_gate_array;
        QJsonObject post_gate;
        post_gate.insert("$type", "app.bsky.feed.postgate#disableRule");
        post_gate_array.append(post_gate);
        part.insert("postgateEmbeddingRules", post_gate_array);
    }
    if (thread_gate_type == "everybody") {
        // nop
    } else {
        QJsonArray thread_gate_array;
        if (thread_gate_type == "choice") {
            for (const auto &option : thread_gate_options) {
                QJsonObject thread_gate;
                if (option == "mentioned") {
                    thread_gate.insert("$type", "app.bsky.feed.threadgate#mentionRule");
                    thread_gate_array.append(thread_gate);
                } else if (option == "followed") {
                    thread_gate.insert("$type", "app.bsky.feed.threadgate#followingRule");
                    thread_gate_array.append(thread_gate);
                } else if (option == "follower") {
                    thread_gate.insert("$type", "app.bsky.feed.threadgate#followerRule");
                    thread_gate_array.append(thread_gate);
                } else if (option.startsWith("at://")) {
                    thread_gate.insert("$type", "app.bsky.feed.threadgate#listRule");
                    thread_gate.insert("list", option);
                    thread_gate_array.append(thread_gate);
                }
            }
        } else {
            // nobodyとかは空配列
        }
        part.insert("threadgateAllowRules", thread_gate_array);
    }

    return part;
}

}
