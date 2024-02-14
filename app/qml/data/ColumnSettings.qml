import QtQuick 2.15

QtObject {
    property bool autoLoading: false
    property int loadingInterval: 300000
    property int imageLayoutType: 1
    property string columnName: ""
    property string columnValue: ""

    property bool visibleLike: true
    property bool visibleRepost: true
    property bool visibleFollow: true
    property bool visibleMention: true
    property bool visibleReply: true
    property bool visibleQuote: true
    property bool visibleReplyToUnfollowedUsers: true

    // 保存しない全体からの反映
    property bool updateSeenNotification: true
    property bool sequentialDisplayOfPosts: true
    property int displayInterval: sequentialDisplayOfPosts ? 400 : 0
}
