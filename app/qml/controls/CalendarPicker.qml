import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.controls.calendartablemodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"

Frame {
    id: root

    property string target: ""  // since or until
    property alias since: calendarTableModel.since
    property alias until: calendarTableModel.until
    property alias enableSince: calendarTableModel.enableSince
    property alias enableUntil: calendarTableModel.enableUntil

    signal dateChanged(int year, int month, int day)

    function setSince(year, month, day) {
        calendarTableModel.setSinceDate(year, month, day)
    }
    function setUntil(year, month, day) {
        calendarTableModel.setUntilDate(year, month, day)
    }
    function sinceUtc() {
        return calendarTableModel.sinceUtc()
    }
    function untilUtc() {
        return calendarTableModel.untilUtc()
    }
    function clear() {
        calendarTableModel.clear()
    }
    function forceLayout(){
        calendarTable.forceLayout()
    }
    onTargetChanged: {
        if(target === "since"){
            calendarTableModel.setCurrentFromSince()
        }else{
            calendarTableModel.setCurrentFromUntil()
        }
    }

    ColumnLayout {
        RowLayout {
            id: controlLayout
            Layout.alignment: Qt.AlignHCenter
            spacing: 0
            property int buttonSize: yearMonthLabel.height * 1.5
            IconButton {
                Layout.preferredWidth: parent.buttonSize
                Layout.preferredHeight: parent.buttonSize
                iconSource: "../images/arrow_left_double.png"
                onClicked: calendarTableModel.addYears(-1)
            }
            IconButton {
                Layout.preferredWidth: parent.buttonSize
                Layout.preferredHeight: parent.buttonSize
                Layout.rightMargin: 5
                iconSource: "../images/arrow_left_single.png"
                onClicked: calendarTableModel.addMonths(-1)
            }
            Label {
                id: yearMonthLabel
                font.pointSize: AdjustedValues.f10
                text: calendarTableModel.year + "/" + calendarTableModel.month
            }
            IconButton {
                Layout.preferredWidth: parent.buttonSize
                Layout.preferredHeight: parent.buttonSize
                Layout.leftMargin: 5
                iconSource: "../images/arrow_right_single.png"
                onClicked: calendarTableModel.addMonths(1)
            }
            IconButton {
                Layout.preferredWidth: parent.buttonSize
                Layout.preferredHeight: parent.buttonSize
                iconSource: "../images/arrow_right_double.png"
                onClicked: calendarTableModel.addYears(1)
            }
        }
        TableView {
            id: calendarTable
            Layout.preferredWidth: 200 * AdjustedValues.ratio
            Layout.preferredHeight: 150 * AdjustedValues.ratio
            clip: true
            columnSpacing: 0
            rowSpacing: 0
            delegate: Button {
                width: implicitWidth
                height: implicitHeight
                implicitWidth: calendarTable.width / 7
                implicitHeight: calendarTable.height / 6
                topInset: 0
                bottomInset: 0
                leftInset: 0
                rightInset: 0
                flat: true
                visible: model.day.length > 0
                Label {
                    anchors.centerIn: parent
                    font.pointSize: AdjustedValues.f10
                    text: model.day
                    color: !model.outOfRange ? Material.color(Material.Grey) :
                                               (model.isCurrentMonth ? Material.foreground : Material.color(Material.BlueGrey))
                }
                onClicked: dateChanged(model.year, model.month, model.day)
            }
            model: CalendarTableModel {
                id: calendarTableModel
            }
        }
    }
}
