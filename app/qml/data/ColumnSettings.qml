import QtQuick 2.15

QtObject {
    property bool autoLoading: false
    property int loadingInterval: 300000
    property string columnName: ""
    property string columnValue: ""

    property bool visibleLike: true
    property bool visibleRepost: true
    property bool visibleFollow: true
    property bool visibleMention: true
    property bool visibleReply: true
    property bool visibleQuote: true
    property bool visibleReplyToUnfollowedUsers: true
}
