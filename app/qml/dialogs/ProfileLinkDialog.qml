import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.externallink 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../compat"

Dialog {
    id: profileLinkDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: editMode ? qsTr("Edit link") : qsTr("Add link")
    closePolicy: Popup.NoAutoClose

    // 入力（編集時の初期値）と結果
    property bool editMode: false
    property string url: ""
    property string linkTitle: ""
    property string icon: ""        // 表示用（既存のアイコンURL or ローカル）
    property string iconLocal: ""   // 検証で取得したアイコン（ローカル）
    property bool iconChanged: false

    property bool verified: false
    property string verifiedUrl: ""
    property bool requesting: false

    signal requestDelete()

    onOpened: {
        urlText.text = profileLinkDialog.url
        titleText.text = profileLinkDialog.linkTitle
        iconChanged = false
        iconLocal = ""
        externalLink.clear()
        requesting = false
        verified = editMode
        verifiedUrl = editMode ? profileLinkDialog.url : ""
        statusLabel.text = ""
    }
    onClosed: {
        urlText.clear()
        titleText.clear()
        externalLink.clear()
        statusLabel.text = ""
    }

    Shortcut {  // Close
        enabled: profileLinkDialog.visible && !externalLink.running
        sequence: "Esc"
        onActivated: profileLinkDialog.reject()
    }

    SystemTool {
        id: systemTool
    }
    ExternalLink {
        id: externalLink
        fetchIcon: true
        onRunningChanged: {
            if(running || !profileLinkDialog.requesting){
                return
            }
            profileLinkDialog.requesting = false
            profileLinkDialog.verified = true
            profileLinkDialog.verifiedUrl = urlText.text
            profileLinkDialog.iconLocal = iconLocal
            profileLinkDialog.icon = iconLocal
            profileLinkDialog.iconChanged = true
            if(valid && title.length > 0){
                titleText.text = title
                statusLabel.text = ""
            }else{
                statusLabel.text = qsTr("Could not get the title. Please enter it manually.")
            }
        }
    }

    ColumnLayout {
        spacing: 0
        Label {
            Layout.fillWidth: true
            font.pointSize: AdjustedValues.f10
            text: qsTr("URL") + (urlText.isValid ? "" : qsTr(": Please enter a URL that starts with 'https://'"))
            color: urlText.isValid ? Material.foreground : Material.color(Material.Red)
        }
        RowLayout {
            TextField {
                id: urlText
                Layout.preferredWidth: 400 * AdjustedValues.ratio
                enabled: !externalLink.running
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                placeholderText: "https://"
                property bool isValid: text.length === 0 || text.startsWith("https://")
                onTextChanged: {
                    if(profileLinkDialog.verified && text !== profileLinkDialog.verifiedUrl){
                        // URLを変えたら再検証が必要
                        profileLinkDialog.verified = false
                        statusLabel.text = ""
                    }
                }
            }
            Button {
                id: verifyButton
                font.pointSize: AdjustedValues.f10
                enabled: urlText.text.length > "https://".length &&
                         urlText.text.startsWith("https://") &&
                         !externalLink.running
                text: qsTr("Verify")
                onClicked: {
                    profileLinkDialog.requesting = true
                    externalLink.getExternalLink(urlText.text)
                }
                BusyIndicator {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: externalLink.running
                }
            }
        }
        RowLayout {
            Layout.topMargin: 5
            Label {
                Layout.fillWidth: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Title")
            }
            Label {
                Layout.alignment: Qt.AlignTop
                font.pointSize: AdjustedValues.f8
                text: 40 - titleText.realTextLength
                color: titleText.realTextLength > 40 ? Material.color(Material.Red) : Material.foreground
            }
        }
        RowLayout {
            Image {
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                fillMode: Image.PreserveAspectFit
                property string providerLogo: systemTool.supportProviderLogo(urlText.text)
                source: providerLogo.length > 0 ? "../images/community/" + providerLogo + ".svg"
                                                : profileLinkDialog.icon.length > 0 ? profileLinkDialog.icon
                                                                                    : "../images/link.png"
                sourceSize.height: height
                layer.enabled: providerLogo.length > 0 || profileLinkDialog.icon.length === 0
                layer.effect: ColorOverlayC {
                    color: Material.foreground
                }
            }
            TextField {
                id: titleText
                Layout.fillWidth: true
                enabled: !externalLink.running
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                placeholderText: qsTr("Optional")
                property int realTextLength: systemTool.countText(text)
            }
        }
        Label {
            id: statusLabel
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            visible: text.length > 0
            wrapMode: Text.Wrap
            font.pointSize: AdjustedValues.f8
            color: Material.color(Material.Orange)
        }

        RowLayout {
            Layout.topMargin: 10
            Button {
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: profileLinkDialog.reject()
            }
            Button {
                visible: profileLinkDialog.editMode
                flat: true
                font.pointSize: AdjustedValues.f10
                Material.foreground: Material.color(Material.Red)
                text: qsTr("Delete")
                onClicked: {
                    profileLinkDialog.requestDelete()
                    profileLinkDialog.reject()
                }
            }
            Item {
                Layout.fillWidth: true
                height: 1
            }
            Button {
                id: acceptButton
                font.pointSize: AdjustedValues.f10
                enabled: profileLinkDialog.verified &&
                         urlText.text === profileLinkDialog.verifiedUrl &&
                         titleText.realTextLength <= 40 &&
                         !externalLink.running
                text: profileLinkDialog.editMode ? qsTr("Apply") : qsTr("Add")
                onClicked: {
                    profileLinkDialog.url = urlText.text
                    profileLinkDialog.linkTitle = titleText.text
                    profileLinkDialog.accept()
                }
            }
        }
    }
}
