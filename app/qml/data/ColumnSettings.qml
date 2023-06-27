import QtQuick 2.15

QtObject {
    property bool autoLoading: false
    property int loadingInterval: 300000
    property string columnName: ""
    property string columnValue: ""

    property bool enableLike: true
    property bool enableRepost: true
    property bool enableFollow: true
    property bool enableMention: true
    property bool enableReply: true
    property bool enableQuote: true
}
