import QtQuick 2.15
import QtQuick.Controls 2.15

import "../controls"

MenuEx {
    id: selfLabelPopup
    // メニューをウインドウのセンターに表示する
    // 本当は親のボタン基準で出したいが2回目の表示から画面外にはみ出ないようにする
    // 制御が正しく働かないようではみ出てしまうので暫定対策
    // ポストダイアログがウインドウ中央に表示しているのでそこまで違和感は無し
    anchors.centerIn: Overlay.overlay

    signal triggered(string value, string text)

    Action {
        text: qsTr("Sexually Explicit")
        property string value: "porn"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    Action {
        text: qsTr("Nudity")
        property string value: "nudity"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    Action {
        text: qsTr("Sexually Suggestive")
        property string value: "sexual"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    Action {
        text: qsTr("Gore")
        property string value: "graphic-media"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    MenuSeparator { }
    Action {
        text: qsTr("Remove")
        property string value: ""
        onTriggered: selfLabelPopup.triggered(value, text)
    }
}
