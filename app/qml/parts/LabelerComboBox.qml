import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.moderation.labelerlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../data"

ComboBox {
    id: labelerDidComboBox
    textRole: "title"
    valueRole: "did"

    property alias service: labelerListModel.service
    property alias handle: labelerListModel.handle
    property alias accessJwt: labelerListModel.accessJwt

    signal errorOccured(string code, string message)
    function load(){
        labelerListModel.load()
    }

    model: LabelerListModel {
        id: labelerListModel
        onFinished: {
            if(rowCount() > 0){
                labelerDidComboBox.currentIndex = 0
            }
        }

        onErrorOccured: (code, message) => labelerDidComboBox.errorOccured(code, message)
    }
    delegate: ItemDelegate {
        width: labelerDidComboBox.width
        height: implicitHeight * AdjustedValues.ratio
        font.pointSize: AdjustedValues.f10
        onClicked: labelerDidComboBox.currentIndex = model.index
        ColumnLayout {
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.verticalCenter: parent.verticalCenter
            spacing: 3
            Label {
                font.pointSize: AdjustedValues.f10
                text: model.title
            }
            Label {
                font.pointSize: AdjustedValues.f8
                text: model.description.split("\n")[0]
            }
        }
    }
    BusyIndicator {
        anchors.fill: parent
        visible: labelerListModel.running
    }
}
