#include "globalsettings.h"
#include "realtime/firehosereceiver.h"
#include "qtquick/systemtool.h"

#include <QSettings>
#include <QFont>
#include <QFontDatabase>

void setAppFont(QGuiApplication &app, QSettings &settings)
{
    QString family = settings.value("fontFamily").toString();
    if (family.isEmpty()) {
        family = SystemTool::defaultFontFamily();
    }
    if (QFontDatabase::families().contains(family)) {
        app.setFont(QFont(family));
        if (settings.value("fontFamily").toString() != family) {
            settings.setValue("fontFamily", family);
        }
    }
}

void setRealtimeFeedEndpoint(QSettings &settings)
{
    RealtimeFeed::FirehoseReceiver *receiver = RealtimeFeed::FirehoseReceiver::getInstance();
    if (receiver == nullptr)
        return;
    if (!settings.contains("realtimeServiceEndpoint")) {
        // キーが無い状態で起動するとなぜか翻訳のキーが消えてしまうので、ここで設定する
        settings.setValue("realtimeServiceEndpoint", "wss://jetstream1.us-west.bsky.network");
    }
    QString endpoint = settings.value("realtimeServiceEndpoint").toString();
    qDebug() << "Load realtime feed endpoint :" << endpoint;
    receiver->setServiceEndpoint(endpoint);
}

void setDefaultValue(QSettings &settings, const QString &key, const QVariant &value)
{
    if (!settings.contains(key)) {
        qDebug() << "Set default value:" << key << value;
        settings.setValue(key, value);
    }
}

void setDefaultSettings(QSettings &settings)
{
    // General
    setDefaultValue(settings, "theme", 0);
    // setDefaultValue(settings, "accent", 0);
    setDefaultValue(settings, "fontSizeRatio", 1.0);
    setDefaultValue(settings, "fontFamily", QString());
    setDefaultValue(settings, "maximumFlickVelocity", 2500);
    setDefaultValue(settings, "wheelDeceleration", 10000);
    setDefaultValue(settings, "language", QString());
    // Feed
    setDefaultValue(settings, "displayOfPosts", QStringLiteral("sequential"));
    setDefaultValue(settings, "updateSeenNotification", true);
    setDefaultValue(settings, "realtimeServiceEndpoint",
                    QStringLiteral("wss://jetstream1.us-west.bsky.network"));
    // Notification
    setDefaultValue(settings, "enableChatNotification", true);
    setDefaultValue(settings, "enableNotificationsForReactionsOnReposts", true);
    // Layout
    setDefaultValue(settings, "rowCount", 1);
    setDefaultValue(settings, "rowHeightRatio2", 50);
    setDefaultValue(settings, "rowHeightRatio31", 35);
    setDefaultValue(settings, "rowHeightRatio32", 70);
    setDefaultValue(settings, "imageLayoutType", 1);
    setDefaultValue(settings, "autoHideDetailMode", true);
    // Translate
    setDefaultValue(settings, "translateApiUrl",
                    QStringLiteral("https://api-free.deepl.com/v2/translate"));
    setDefaultValue(settings, "translateApiKey", QString());
    setDefaultValue(settings, "translateTargetLanguage", QStringLiteral("JA"));
    // About
    setDefaultValue(settings, "displayRealtimeFeedStatus", false);
    setDefaultValue(settings, "displayVersionInfoInMainArea", true);
}

void setGlobalSettings(QGuiApplication &app)
{
    QSettings settings;
    // qDebug() << "1" << settings.allKeys();
    setAppFont(app, settings);
    // qDebug() << "2" << settings.allKeys();
    setRealtimeFeedEndpoint(settings);
    // qDebug() << "3" << settings.allKeys();
    setDefaultSettings(settings);
    // qDebug() << "4" << settings.allKeys();
    SystemTool::setFlicableWheelDeceleration(settings.value("wheelDeceleration", 10000).toInt());
}
