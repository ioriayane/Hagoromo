import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.singleton 1.0

ComboBox {
    id: control
    font.pointSize: AdjustedValues.f10
    textRole: "text"
    valueRole: "value"
    delegate: ItemDelegate {
        width: control.width
        height: implicitHeight * AdjustedValues.ratio
        font.pointSize: AdjustedValues.f10
        text: model.text
        onClicked: control.currentIndex = model.index
    }
    function setByValue(value){
        currentIndex = indexOfValue(value)
    }
}
