import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    id: root

    // 進捗アイテムの表示要求（itemId が既存の場合は更新、なければ追加）
    function notify(itemId, headerText, message) {
        for (var i = 0; i < progressModel.count; i++) {
            if (progressModel.get(i).itemId === itemId) {
                progressModel.setProperty(i, "headerText", headerText)
                progressModel.setProperty(i, "message", message)
                return
            }
        }
        progressModel.append({itemId: itemId, headerText: headerText, message: message})
    }

    // 進捗アイテムの削除要求
    function clear(itemId) {
        for (var i = 0; i < progressModel.count; i++) {
            if (progressModel.get(i).itemId === itemId) {
                progressModel.remove(i)
                return
            }
        }
    }

    ListModel {
        id: progressModel
    }

    Repeater {
        model: progressModel
        delegate: OperationProgressFrame {
            Layout.alignment: Qt.AlignRight
            headerText: model.headerText
            message: model.message
        }
    }
}
