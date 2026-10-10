import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    id: root

    readonly property int maxCount: 5

    // クリックされたフレームの内容を外部に伝える
    signal messageClicked(string headerText, string message)

    // エラーメッセージの表示要求（末尾に追加、最大保持数を超えたら一番古いものを削除）
    function notify(headerText, message) {
        errorModel.append({headerText: headerText, message: message})
        if(errorModel.count > root.maxCount){
            errorModel.remove(0)
        }
    }

    ListModel {
        id: errorModel
    }

    Repeater {
        model: errorModel
        delegate: ErrorNotificationFrame {
            Layout.alignment: Qt.AlignRight
            headerText: model.headerText
            message: model.message
            onClicked: {
                root.messageClicked(headerText, message)
                errorModel.remove(index)
            }
            onCloseClicked: errorModel.remove(index)
        }
    }
}
