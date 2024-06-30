import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as P

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../parts"

Dialog {
    id: searchDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Popup.NoAutoClose

    property int parentWidth: parent.width
    property alias accountModel: accountCombo.model
    property alias selectedAccountIndex: accountCombo.currentIndex

    property string defaultAccountUuid: ""
    property string searchType: "posts"
    property string searchText: ""

    onOpened: {
        var i = accountModel.indexAt(defaultAccountUuid)
        accountCombo.currentIndex = -1
        if(i >= 0){
            accountCombo.currentIndex = i
        } else {
            accountCombo.currentIndex = accountModel.getMainAccountIndex()
        }
        searchTextField.forceActiveFocus()
    }
    onClosed: {
        defaultAccountUuid = ""
        searchType = "posts"
        searchText = ""
        searchTextField.clear()
        byMeCheckBox.checked = false
        sinceCheckBox.checked = false
        untilCheckBox.checked = false
        calendarPicker.clear()
    }

    Shortcut {  // Post
        enabled: searchDialog.visible && postButton.enabled && searchTextField.focus
        sequence: "Ctrl+Return"
        onActivated: postButton.clicked()
    }
    Shortcut {  // Close
        // DialogのclosePolicyでEscで閉じられるけど、そのうち編集中の確認ダイアログを
        // 入れたいので別でイベント処理をする。onClosedで閉じるをキャンセルできなさそうなので。
        enabled: searchDialog.visible && ! postButton.enabled
        sequence: "Esc"
        onActivated: searchDialog.close()
    }

    ButtonGroup {
        id: searchTypeButtonGroup
        buttons: searchTypeRowlayout.children
    }

    ColumnLayout {
        spacing: AdjustedValues.s5

        ComboBox {
            id: accountCombo
            Layout.preferredWidth: 200 * AdjustedValues.ratio + AdjustedValues.i24
            Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
            font.pointSize: AdjustedValues.f10
            textRole: "handle"
            valueRole: "did"
            delegate: ItemDelegate {
                width: parent.width
                height: implicitHeight * AdjustedValues.ratio
                font.pointSize: AdjustedValues.f10
                onClicked: accountCombo.currentIndex = model.index
                AccountLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    source: model.avatar
                    handle: model.handle
                }
            }
            contentItem: AccountLayout {
                id: accountAvatarLayout
                width: parent.width
                height: parent.height
                leftMargin: 10
                handle: accountCombo.displayText
            }
            onCurrentIndexChanged: {
                var row = accountCombo.currentIndex
                if(row >= 0){
                    accountAvatarLayout.source =
                            postDialog.accountModel.item(row, AccountListModel.AvatarRole)
                }
            }
        }


        RowLayout {
            id: searchTypeRowlayout
            RadioButton {
                property string value: "posts"
                font.pointSize: AdjustedValues.f10
                text: qsTr("Posts")
                checked: true
            }
            RadioButton {
                property string value: "users"
                font.pointSize: AdjustedValues.f10
                text: qsTr("Users")
            }
        }

        TextField  {
            id: searchTextField
            Layout.preferredWidth: 350 * AdjustedValues.ratio
            selectByMouse: true
            font.pointSize: AdjustedValues.f10
        }

        RowLayout {
            id: detailConditionsLayout
            visible: (searchTypeButtonGroup.checkedButton.value === "posts")
            spacing: 0
            CheckBox {
                id: byMeCheckBox
                topPadding: 5
                bottomPadding: 5
                font.pointSize: AdjustedValues.f8
                text: qsTr("by me")
            }
            CheckBox {
                id: sinceCheckBox
                topPadding: 5
                bottomPadding: 5
                rightPadding: 0
                font.pointSize: AdjustedValues.f8
                text: qsTr("Since") + ":"
            }
            Button {
                id: sinceButton
                topInset: 0
                bottomInset: 0
                leftPadding: 3
                rightPadding: 3
                flat: true
                enabled: sinceCheckBox.checked
                font.pointSize: AdjustedValues.f8
                text: calendarPicker.since
                onClicked: {
                    calendarPicker.target = "since"
                    calendarPickerPopup.x = x
                    calendarPickerPopup.y = y
                    calendarPickerPopup.open()
                }
            }
            CheckBox {
                id: untilCheckBox
                topPadding: 5
                bottomPadding: 5
                rightPadding: 0
                font.pointSize: AdjustedValues.f8
                text: qsTr("Until") + ":"
            }
            Button {
                id: untilButton
                topInset: 0
                bottomInset: 0
                leftPadding: 3
                rightPadding: 3
                flat: true
                enabled: untilCheckBox.checked
                font.pointSize: AdjustedValues.f8
                text: calendarPicker.until
                onClicked: {
                    calendarPicker.target = "until"
                    calendarPickerPopup.x = x
                    calendarPickerPopup.y = y
                    calendarPickerPopup.open()
                }
            }
            Popup {
                id: calendarPickerPopup
                onOpened: calendarPicker.forceLayout()
                CalendarPicker {
                    id: calendarPicker
                    enableSince: sinceCheckBox.checked
                    enableUntil: untilCheckBox.checked
                    onDateChanged: (year, month, day) => {
                                       if(target === "since"){
                                           calendarPicker.setSince(year, month, day)
                                       }else{
                                           calendarPicker.setUntil(year, month, day)
                                       }
                                       target = ""
                                       calendarPickerPopup.close()
                                   }
                }
            }
        }

        RowLayout {
            //            Layout.alignment: Qt.AlignRight
            Button {
                font.pointSize: AdjustedValues.f10
                flat: true
                text: qsTr("Cancel")
                onClicked: searchDialog.close()
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                id: postButton
                Layout.alignment: Qt.AlignRight
                enabled: searchTextField.text.length > 0
                font.pointSize: AdjustedValues.f10
                text: qsTr("Search")
                onClicked: {
                    searchDialog.searchType = searchTypeButtonGroup.checkedButton.value
                    searchDialog.searchText = searchTextField.text
                    if(searchDialog.searchType === "posts"){
                        searchDialog.searchText += byMeCheckBox.checked ? " from:me" : ""
                        searchDialog.searchText += sinceCheckBox.checked ? (" since:" + calendarPicker.sinceUtc()) : ""
                        searchDialog.searchText += untilCheckBox.checked ? (" until:" + calendarPicker.untilUtc()) : ""
                    }
                    searchDialog.accept()
                }
            }
        }
    }
}
