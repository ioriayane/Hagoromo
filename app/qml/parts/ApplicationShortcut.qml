import QtQuick 2.15

Item {
    id: item
    property alias postDialogShortcut: postDialogShortcut
    property alias searchDialogShortcut: searchDialogShortcut

    signal showColumn(int index)  // -1:一番右、1~9

    // Post
    Shortcut {
        id: postDialogShortcut
        enabled: item.enabled
        context: Qt.ApplicationShortcut
        sequence: "n"
    }
    // Search
    Shortcut {
        id: searchDialogShortcut
        enabled: item.enabled
        context: Qt.ApplicationShortcut
        sequence: "s"
    }
    // カラムの移動
    Shortcut { enabled: item.enabled; context: Qt.ApplicationShortcut; sequence: "1"; onActivated: showColumn(1) }
    Shortcut { enabled: item.enabled; context: Qt.ApplicationShortcut; sequence: "2"; onActivated: showColumn(2) }
    Shortcut { enabled: item.enabled; context: Qt.ApplicationShortcut; sequence: "3"; onActivated: showColumn(3) }
    Shortcut { enabled: item.enabled; context: Qt.ApplicationShortcut; sequence: "4"; onActivated: showColumn(4) }
    Shortcut { enabled: item.enabled; context: Qt.ApplicationShortcut; sequence: "5"; onActivated: showColumn(5) }
    Shortcut { enabled: item.enabled; context: Qt.ApplicationShortcut; sequence: "6"; onActivated: showColumn(6) }
    Shortcut { enabled: item.enabled; context: Qt.ApplicationShortcut; sequence: "7"; onActivated: showColumn(7) }
    Shortcut { enabled: item.enabled; context: Qt.ApplicationShortcut; sequence: "8"; onActivated: showColumn(8) }
    Shortcut { enabled: item.enabled; context: Qt.ApplicationShortcut; sequence: "9"; onActivated: showColumn(9) }
    // 一番左
    Shortcut {
        enabled: item.enabled
        context: Qt.ApplicationShortcut
        sequence: "ctrl+left"
        onActivated: showColumn(1)
    }
    // 一番右
    Shortcut {
        enabled: item.enabled
        context: Qt.ApplicationShortcut
        sequence: "ctrl+right"
        onActivated: showColumn(-1)
    }
}
