import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    id: root

    // 進捗アイテムの表示要求
    // itemId + contentId の組み合わせが既存の場合は更新、なければ追加
    function notify(itemId, contentId, headerText, message) {
        for (var i = 0; i < progressModel.count; i++) {
            var entry = progressModel.get(i)
            if (entry.itemId === itemId && entry.contentId === contentId) {
                progressModel.setProperty(i, "headerText", headerText)
                progressModel.setProperty(i, "message", message)
                return
            }
        }
        progressModel.append({itemId: itemId, contentId: contentId, headerText: headerText, message: message})
    }

    // 進捗アイテムの削除要求（itemId + contentId で特定の1件を削除）
    function clear(itemId, contentId) {
        for (var i = progressModel.count - 1; i >= 0; i--) {
            var entry = progressModel.get(i)
            if (entry.itemId === itemId && entry.contentId === contentId) {
                progressModel.remove(i)
                return
            }
        }
    }

    // 進捗アイテムの削除要求（itemId に紐づくすべての件を削除）
    function clearAll(itemId) {
        for (var i = progressModel.count - 1; i >= 0; i--) {
            if (progressModel.get(i).itemId === itemId) {
                progressModel.remove(i)
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
