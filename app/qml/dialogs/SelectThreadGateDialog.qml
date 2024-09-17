import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.listslistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: selectThreadGateDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Post interaction settings") + (defaultSettingMode ? qsTr("(default value)") : "")

    bottomPadding: AdjustedValues.s10

    property bool defaultSettingMode: false
    property bool ready: true   // 後から変更する時はfalseにしてAcceptさせないようにする

    property bool initialQuoteEnabled: true
    property string initialType: "everybody"
    property variant initialOptions: []
    property bool selectedQuoteEnabled: true
    property string selectedType: "everybody"
    property variant selectedOptions: []

    property alias account: account
    Account {
        id: account
    }
    signal errorOccured(string account_uuid, string code, string message)

    onInitialQuoteEnabledChanged: quoteEanbled.checked = initialQuoteEnabled
    onOpened: {
        var i
        choiceRadioButton.checked = true
        for(i=0; i<group.buttons.length; i++){
            group.buttons[i].checked = (group.buttons[i].value === initialType)
        }
        mentionedCheckBox.checked = false
        followedCheckBox.checked = false
        for(i=0; i<initialOptions.length; i++){
            if(initialOptions[i] === "mentioned"){
                mentionedCheckBox.checked = true
            }else if(initialOptions[i] === "followed"){
                followedCheckBox.checked = true
            }else{
                listsListModel.setInitialSelected(initialOptions[i])
            }
        }
        listsListModel.clear()
        listsListModel.setAccount(account.service, account.did, account.handle,
                                  account.email, account.accessJwt, account.refreshJwt)
        listsListModel.getLatest()
    }
    onClosed: {
        quoteEanbled.checked = true
        listsListModel.clear()
        var i
        for(i=0; i<group.buttons.length; i++){
            group.buttons[i].checked = false
        }
        choiceRadioButton.checked = false
        mentionedCheckBox.checked = false
        followedCheckBox.checked = false
    }

    function clear(){
        selectedQuoteEnabled = true
        selectedType = "everybody"
        selectedOptions = []
    }

    ButtonGroup {
        id: group
        onClicked: (button) => console.log("" + button.value)
    }

    ColumnLayout {
        spacing: AdjustedValues.s10
        Label {
            font.pointSize: AdjustedValues.f10
            bottomPadding: 2
            text: qsTr("Quote settings")
        }
        Switch {
            id: quoteEanbled
            verticalPadding: 0
            font.pointSize: AdjustedValues.f10
            enabled: selectThreadGateDialog.ready
            text: qsTr("Quote posts enabled")
        }
        Label {
            font.pointSize: AdjustedValues.f10
            bottomPadding: 2
            text: qsTr("Reply settings")
        }
        RadioButton {
            ButtonGroup.group: group
            verticalPadding: 3
            font.pointSize: AdjustedValues.f10
            text: qsTr("Everybody")
            property string value: "everybody"
        }
        RadioButton {
            ButtonGroup.group: group
            verticalPadding: 3
            font.pointSize: AdjustedValues.f10
            text: qsTr("Nobody")
            property string value: "nobody"
        }
        GroupBox {
            label: RadioButton {
                id: choiceRadioButton
                ButtonGroup.group: group
                topPadding: 5
                bottomPadding: 5
                font.pointSize: AdjustedValues.f10
                text: qsTr("Combine these options")
                property string value: "choice"
            }

            ColumnLayout {
                id: choiceLayout
                anchors.fill: parent
                enabled: choiceRadioButton.checked
                spacing: 0 //AdjustedValues.s20

                property int checkedCount: {
                    var count = 0
                    count += mentionedCheckBox.checked ? 1 : 0
                    count += followedCheckBox.checked ? 1 : 0
                    count += listsListModel.checkedCount
                    return count
                }

                CheckBox {
                    id: mentionedCheckBox
                    leftPadding: 15
                    topPadding: 3
                    bottomPadding: AdjustedValues.s10
                    font.pointSize: AdjustedValues.f10
                    enabled: checked || choiceLayout.checkedCount < 5
                    text: qsTr("Mentioned users")
                    property string value: "mentioned"
                }
                CheckBox {
                    id: followedCheckBox
                    leftPadding: 15
                    topPadding: AdjustedValues.s10
                    bottomPadding: AdjustedValues.s10
                    font.pointSize: AdjustedValues.f10
                    enabled: checked || choiceLayout.checkedCount < 5
                    text: qsTr("Followed users")
                    property string value: "followed"
                }

                ScrollView {
                    id: listsListScroll
                    Layout.preferredWidth: 350 * AdjustedValues.ratio
                    Layout.preferredHeight: 150 * AdjustedValues.ratioHalf
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    clip: true

                    ListView {
                        id: rootListView
                        anchors.fill: parent
                        maximumFlickVelocity: AdjustedValues.maximumFlickVelocity
                        model: ListsListModel {
                            id: listsListModel
                            actor: account.did
                            visibilityType: ListsListModel.VisibilityTypeCuration
                            property variant initialSelected: []

                            function setInitialSelected(uri){
                                initialSelected[uri] = true
                            }
                        }
                        onMovementEnded: {
                            if(atYEnd){
                                listsListModel.getNext()
                            }
                        }

                        footer:  Item {
                            width: rootListView.width - listsListScroll.ScrollBar.vertical.width
                            height: AdjustedValues.b24
                            BusyIndicator {
                                id: busyIndicator
                                anchors.centerIn: parent
                                height: AdjustedValues.i32
                                visible: listsListModel ? listsListModel.running : false
                            }
                        }
                        delegate: CheckBox {
                            verticalPadding: AdjustedValues.s10
                            leftPadding: 15
                            font.pointSize: AdjustedValues.f10
                            enabled: checked || choiceLayout.checkedCount < 5
                            text: qsTr("Users in \"%1\"").replace("%1", model.name)
                            property string value: model.uri
                            Component.onCompleted: {
                                if(listsListModel.initialSelected[model.uri]){
                                    checked = true
                                }else{
                                    checked = false
                                }
                                delete listsListModel.initialSelected[model.uri]
                            }
                            onCheckedChanged: listsListModel.update(model.index, ListsListModel.CheckedRole, checked)
                        }
                    }
                }
            }
        }
        RowLayout {
            Button {
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: {
                    selectThreadGateDialog.selectedQuoteEnabled = selectThreadGateDialog.initialQuoteEnabled
                    selectThreadGateDialog.selectedType = selectThreadGateDialog.initialType
                    selectThreadGateDialog.selectedOptions = selectThreadGateDialog.initialOptions
                    selectThreadGateDialog.reject()
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Apply")
                enabled: selectThreadGateDialog.ready
                onClicked: {
                    selectThreadGateDialog.selectedQuoteEnabled = quoteEanbled.checked
                    selectThreadGateDialog.selectedType = "everybody"
                    selectThreadGateDialog.selectedOptions = []
                    var i
                    for(i=0; i<group.buttons.length; i++){
                        if(group.buttons[i].checked){
                            selectThreadGateDialog.selectedType = group.buttons[i].value
                            break
                        }
                    }
                    if(selectThreadGateDialog.selectedType === "choice"){
                        var o_i = 0
                        if(mentionedCheckBox.checked){
                            selectThreadGateDialog.selectedOptions[o_i++] = "mentioned"
                        }
                        if(followedCheckBox.checked){
                            selectThreadGateDialog.selectedOptions[o_i++] = "followed"
                        }
                        for(i=0; i<rootListView.count; i++){
                            if(rootListView.itemAtIndex(i).checked){
                                selectThreadGateDialog.selectedOptions[o_i++] = rootListView.itemAtIndex(i).value
                            }
                        }
                    }
                    selectThreadGateDialog.accept()
                }
            }
        }
    }
}
