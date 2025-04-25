#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QTranslator>
#include <QtQuickControls2/QQuickStyle>

#include "qtquick/account/createsession.h"
#include "qtquick/operation/recordoperator.h"
#include "qtquick/account/accountlistmodel.h"
#include "qtquick/column/columnlistmodel.h"
#include "qtquick/notification/notificationlistmodel.h"
#include "qtquick/timeline/postthreadlistmodel.h"
#include "qtquick/timeline/timelinelistmodel.h"
#include "qtquick/timeline/authorfeedlistmodel.h"
#include "qtquick/timeline/anyfeedlistmodel.h"
#include "qtquick/timeline/actorlikelistmodel.h"
#include "qtquick/timeline/quotedpostlistmodel.h"
#include "qtquick/profile/followslistmodel.h"
#include "qtquick/profile/followerslistmodel.h"
#include "qtquick/profile/knownfollowerslistmodel.h"
#include "qtquick/profile/blockslistmodel.h"
#include "qtquick/profile/muteslistmodel.h"
#include "qtquick/timeline/searchpostlistmodel.h"
#include "qtquick/profile/searchprofilelistmodel.h"
#include "qtquick/column/feedtypelistmodel.h"
#include "qtquick/timeline/customfeedlistmodel.h"
#include "qtquick/feedgenerator/feedgeneratorlistmodel.h"
#include "qtquick/feedgenerator/actorfeedgeneratorlistmodel.h"
#include "qtquick/controls/calendartablemodel.h"
#include "qtquick/controls/embedimagelistmodel.h"
#include "qtquick/controls/emojilistmodel.h"
#include "qtquick/controls/languagelistmodel.h"
#include "qtquick/moderation/contentfiltersettinglistmodel.h"
#include "qtquick/moderation/mutedwordlistmodel.h"
#include "qtquick/profile/anyprofilelistmodel.h"
#include "qtquick/list/listslistmodel.h"
#include "qtquick/list/listitemlistmodel.h"
#include "qtquick/list/listfeedlistmodel.h"
#include "qtquick/list/listblockslistmodel.h"
#include "qtquick/list/listmuteslistmodel.h"
#include "qtquick/thumbnailprovider.h"
#include "qtquick/profile/userprofile.h"
#include "qtquick/timeline/userpost.h"
#include "qtquick/systemtool.h"
#include "qtquick/link/externallink.h"
#include "qtquick/moderation/reporter.h"
#include "qtquick/link/feedgeneratorlink.h"
#include "qtquick/link/listlink.h"
#include "qtquick/link/postlink.h"
#include "qtquick/log/logoperator.h"
#include "qtquick/log/logstatisticslistmodel.h"
#include "qtquick/log/logdailylistmodel.h"
#include "qtquick/log/logmonthlylistmodel.h"
#include "qtquick/log/logfeedlistmodel.h"
#include "qtquick/chat/chatlistmodel.h"
#include "qtquick/chat/chatmessagelistmodel.h"
#include "qtquick/chat/chatnotificationlistmodel.h"
#include "qtquick/moderation/labelerlistmodel.h"
#include "qtquick/blog/blogentrylistmodel.h"
#include "qtquick/realtime/realtimefeedlistmodel.h"
#include "qtquick/realtime/editselectorlistmodel.h"
#include "qtquick/realtime/realtimefeedstatuslistmodel.h"
#include "qtquick/realtime/realtimefeedstatusgraph.h"

#include "tools/globalsettings.h"
#include "tools/encryption.h"
#include "tools/translatorchanger.h"

int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Material");

    app.setOrganizationName(QStringLiteral("relog"));
    app.setOrganizationDomain(QStringLiteral("hagoromo.relog.tech"));
    app.setApplicationName(QStringLiteral("Hagoromo"));
    app.setApplicationVersion(QStringLiteral("0.48.0"));
#ifndef HAGOROMO_RELEASE_BUILD
    app.setApplicationVersion(app.applicationVersion() + "d");
#endif

    qmlRegisterType<CreateSession>("tech.relog.hagoromo.createsession", 1, 0, "CreateSession");
    qmlRegisterType<RecordOperator>("tech.relog.hagoromo.recordoperator", 1, 0, "RecordOperator");
    qmlRegisterType<AccountListModel>("tech.relog.hagoromo.accountlistmodel", 1, 0,
                                      "AccountListModel");
    qmlRegisterType<ColumnListModel>("tech.relog.hagoromo.columnlistmodel", 1, 0,
                                     "ColumnListModel");
    qmlRegisterType<NotificationListModel>("tech.relog.hagoromo.notificationlistmodel", 1, 0,
                                           "NotificationListModel");
    qmlRegisterType<PostThreadListModel>("tech.relog.hagoromo.postthreadlistmodel", 1, 0,
                                         "PostThreadListModel");
    qmlRegisterType<TimelineListModel>("tech.relog.hagoromo.timelinelistmodel", 1, 0,
                                       "TimelineListModel");
    qmlRegisterType<AuthorFeedListModel>("tech.relog.hagoromo.authorfeedlistmodel", 1, 0,
                                         "AuthorFeedListModel");
    qmlRegisterType<AnyFeedListModel>("tech.relog.hagoromo.anyfeedlistmodel", 1, 0,
                                      "AnyFeedListModel");
    qmlRegisterType<ActorLikeListModel>("tech.relog.hagoromo.actorlikelistmodel", 1, 0,
                                        "ActorLikeListModel");
    qmlRegisterType<FollowsListModel>("tech.relog.hagoromo.followslistmodel", 1, 0,
                                      "FollowsListModel");
    qmlRegisterType<FollowersListModel>("tech.relog.hagoromo.followerslistmodel", 1, 0,
                                        "FollowersListModel");
    qmlRegisterType<KnownFollowersListModel>("tech.relog.hagoromo.knownfollowerslistmodel", 1, 0,
                                             "KnownFollowersListModel");
    qmlRegisterType<BlocksListModel>("tech.relog.hagoromo.blockslistmodel", 1, 0,
                                     "BlocksListModel");
    qmlRegisterType<MutesListModel>("tech.relog.hagoromo.muteslistmodel", 1, 0, "MutesListModel");
    qmlRegisterType<SearchPostListModel>("tech.relog.hagoromo.searchpostlistmodel", 1, 0,
                                         "SearchPostListModel");
    qmlRegisterType<SearchProfileListModel>("tech.relog.hagoromo.searchprofilelistmodel", 1, 0,
                                            "SearchProfileListModel");
    qmlRegisterType<FeedTypeListModel>("tech.relog.hagoromo.feedtypelistmodel", 1, 0,
                                       "FeedTypeListModel");
    qmlRegisterType<CustomFeedListModel>("tech.relog.hagoromo.customfeedlistmodel", 1, 0,
                                         "CustomFeedListModel");
    qmlRegisterType<FeedGeneratorListModel>("tech.relog.hagoromo.feedgeneratorlistmodel", 1, 0,
                                            "FeedGeneratorListModel");
    qmlRegisterType<ActorFeedGeneratorListModel>("tech.relog.hagoromo.actorfeedgeneratorlistmodel",
                                                 1, 0, "ActorFeedGeneratorListModel");
    qmlRegisterType<QuotedPostListModel>("tech.relog.hagoromo.quotedpostlistmodel", 1, 0,
                                         "QuotedPostListModel");
    qmlRegisterType<ContentFilterSettingListModel>(
            "tech.relog.hagoromo.contentfiltersettinglistmodel", 1, 0,
            "ContentFilterSettingListModel");
    qmlRegisterType<MutedWordListModel>("tech.relog.hagoromo.mutedwordlistmodel", 1, 0,
                                        "MutedWordListModel");
    qmlRegisterType<AnyProfileListModel>("tech.relog.hagoromo.anyprofilelistmodel", 1, 0,
                                         "AnyProfileListModel");
    qmlRegisterType<ListsListModel>("tech.relog.hagoromo.listslistmodel", 1, 0, "ListsListModel");
    qmlRegisterType<ListItemListModel>("tech.relog.hagoromo.listitemlistmodel", 1, 0,
                                       "ListItemListModel");
    qmlRegisterType<ListFeedListModel>("tech.relog.hagoromo.listfeedlistmodel", 1, 0,
                                       "ListFeedListModel");
    qmlRegisterType<ListBlocksListModel>("tech.relog.hagoromo.listblocksListmodel", 1, 0,
                                         "ListBlocksListModel");
    qmlRegisterType<ListMutesListModel>("tech.relog.hagoromo.listmuteslistmodel", 1, 0,
                                        "ListMutesListModel");
    qmlRegisterType<Encryption>("tech.relog.hagoromo.encryption", 1, 0, "Encryption");
    qmlRegisterType<UserProfile>("tech.relog.hagoromo.userprofile", 1, 0, "UserProfile");
    qmlRegisterType<UserPost>("tech.relog.hagoromo.userpost", 1, 0, "UserPost");
    qmlRegisterType<SystemTool>("tech.relog.hagoromo.systemtool", 1, 0, "SystemTool");
    qmlRegisterType<ExternalLink>("tech.relog.hagoromo.externallink", 1, 0, "ExternalLink");
    qmlRegisterType<Reporter>("tech.relog.hagoromo.reporter", 1, 0, "Reporter");
    qmlRegisterType<FeedGeneratorLink>("tech.relog.hagoromo.feedgeneratorlink", 1, 0,
                                       "FeedGeneratorLink");
    qmlRegisterType<ListLink>("tech.relog.hagoromo.listlink", 1, 0, "ListLink");
    qmlRegisterType<PostLink>("tech.relog.hagoromo.postlink", 1, 0, "PostLink");

    qRegisterMetaType<QList<TotalItem>>("QList<TotalItem>");
    qmlRegisterType<LogOperator>("tech.relog.hagoromo.logoperator", 1, 0, "LogOperator");
    qmlRegisterType<LogStatisticsListModel>("tech.relog.hagoromo.logstatisticslistmodel", 1, 0,
                                            "LogStatisticsListModel");
    qmlRegisterType<LogDailyListModel>("tech.relog.hagoromo.logdailylistmodel", 1, 0,
                                       "LogDailyListModel");
    qmlRegisterType<LogMonthlyListModel>("tech.relog.hagoromo.logmonthlylistmodel", 1, 0,
                                         "LogMonthlyListModel");
    qmlRegisterType<LogFeedListModel>("tech.relog.hagoromo.logfeedlistmodel", 1, 0,
                                      "LogFeedListModel");

    qmlRegisterType<ChatListModel>("tech.relog.hagoromo.chatlistmodel", 1, 0, "ChatListModel");
    qmlRegisterType<ChatMessageListModel>("tech.relog.hagoromo.chatmessagelistmodel", 1, 0,
                                          "ChatMessageListModel");
    qmlRegisterType<ChatNotificationListModel>("tech.relog.hagoromo.chatnotificationlistmodel", 1,
                                               0, "ChatNotificationListModel");

    qmlRegisterType<LabelerListModel>("tech.relog.hagoromo.moderation.labelerlistmodel", 1, 0,
                                      "LabelerListModel");
    qmlRegisterType<BlogEntryListModel>("tech.relog.hagoromo.blog.blogentrylistmodel", 1, 0,
                                        "BlogEntryListModel");
    qmlRegisterType<RealtimeFeedListModel>("tech.relog.hagoromo.realtime.realtimefeedlistmodel", 1,
                                           0, "RealtimeFeedListModel");
    qmlRegisterType<EditSelectorListModel>("tech.relog.hagoromo.realtime.editselectorlistmodel", 1,
                                           0, "EditSelectorListModel");
    qmlRegisterType<RealtimeFeedStatusListModel>(
            "tech.relog.hagoromo.realtime.realtimefeedstatuslistmodel", 1, 0,
            "RealtimeFeedStatusListModel");
    qmlRegisterType<RealtimeFeedStatusGraph>("tech.relog.hagoromo.realtime.realtimefeedstatusgraph",
                                             1, 0, "RealtimeFeedStatusGraph");

    qmlRegisterType<CalendarTableModel>("tech.relog.hagoromo.controls.calendartablemodel", 1, 0,
                                        "CalendarTableModel");
    qmlRegisterType<EmbedImageListModel>("tech.relog.hagoromo.controls.embedimagelistmodel", 1, 0,
                                         "EmbedImageListModel");
    qmlRegisterType<EmojiListModel>("tech.relog.hagoromo.controls.emojilistmodel", 1, 0,
                                    "EmojiListModel");
    qmlRegisterType<LanguageListModel>("tech.relog.hagoromo.controls.languagelistmodel", 1, 0,
                                       "LanguageListModel");

    qmlRegisterSingletonType(QUrl("qrc:/Hagoromo/qml/data/AdjustedValues.qml"),
                             "tech.relog.hagoromo.singleton", 1, 0, "AdjustedValues");

    setGlobalSettings(app);

#ifdef QT_NO_DEBUG
    QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "*.info=false\n"
                                     "*.warning=true\n"
                                     "*.critical=true");
#endif

    QQmlApplicationEngine engine;
    TranslatorChanger changer;
    changer.initialize(&app, &engine);
    changer.setBySavedSetting();

    engine.addImageProvider(QStringLiteral("thumbnail"), new ThumbnailProvider);

    const QUrl url("qrc:/Hagoromo/qml/main.qml");
    QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreated, &app,
            [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl)
                    QCoreApplication::exit(-1);
            },
            Qt::QueuedConnection);
    engine.load(url);
    changer.connect();

    return app.exec();
}
