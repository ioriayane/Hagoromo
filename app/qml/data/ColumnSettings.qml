import QtQuick 2.15

QtObject {
    property bool autoLoading: false
    property int loadingInterval: 300000
    property int imageLayoutType: 1
    property string columnName: ""
    property string columnValue: ""
    property var columnValueList: []

    property bool visibleLike: true
    property bool visibleRepost: true
    property bool visibleFollow: true
    property bool visibleMention: true
    property bool visibleReply: true
    property bool visibleQuote: true
    property bool visibleReplyToUnfollowedUsers: true
    property bool visibleRepostOfOwn: true
    property bool visibleRepostOfFollowingUsers: true
    property bool visibleRepostOfUnfollowingUsers: true
    property bool visibleRepostOfMine: true
    property bool visibleRepostByMe: true
    property bool aggregateReactions: true

    // 保存しない全体からの反映
    property bool updateSeenNotification: true
    property bool sequentialDisplayOfPosts: true
    property int displayInterval: sequentialDisplayOfPosts ? 400 : 0
    property bool enableNotificationsForReactionsOnReposts: true
    property bool autoHideDetailMode: true
}
