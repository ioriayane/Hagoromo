import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.singleton 1.0

Frame {
    id: pollOptions
    property alias model: repeater.model
    ColumnLayout {
        id: optionsLayout
        Label {
            elide: Text.ElideRight
            font.pointSize: AdjustedValues.f8
            text: qsTr("Poll")
        }
        Repeater {
            id: repeater
            delegate: Frame {
                Layout.preferredWidth: pollOptions.width - pollOptions.leftPadding - pollOptions.rightPadding
                clip: true
                topPadding: AdjustedValues.s5
                bottomPadding: AdjustedValues.s5
                Label {
                    elide: Text.ElideRight
                    font.pointSize: AdjustedValues.f8
                    text: model.modelData
                }
            }
        }
    }
}
