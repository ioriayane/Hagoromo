import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.draftlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: selectDraftDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Drafts")

    property int parentHeight: parent.height
    property alias account: account
    property int selectedIndex: -1
    property alias draftModel: draftListModel

    Account {
        id: account
    }

    onOpened: {
        console.log("SelectDraftDialog : " + account.uuid)
        draftListView.currentIndex = -1
        draftListModel.clear()
        draftListModel.setAccount(account.uuid)
        draftListModel.getLatest()
    }

    function clear(){
        draftListModel.clear()
    }

    ColumnLayout {
        ScrollView {
            id: draftScrollView
            Layout.preferredWidth: 500 * AdjustedValues.ratio
            Layout.preferredHeight: selectDraftDialog.parentHeight * 0.5
            //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            clip: true

            ListView {
                id: draftListView
                maximumFlickVelocity: AdjustedValues.maximumFlickVelocity
                clip: true
                spacing: AdjustedValues.s5

                onMovementEnded: {
                    if(atYEnd){
                        draftListModel.getNext()
                    }
                }

                model: DraftListModel {
                    id: draftListModel
                }
                footer: BusyIndicator {
                    id: busyIndicator
                    width: draftListView.width - draftScrollView.ScrollBar.vertical.width
                    height: AdjustedValues.i32
                    visible: draftListView.model ? draftListView.model.running : false
                }

                delegate: ClickableFrame {
                    id: contentRootFrame
                    contentWidth: contentRootLayout.implicitWidth
                    contentHeight: contentRootLayout.implicitHeight
                    topPadding: 10
                    leftPadding: 10
                    rightPadding: 10
                    bottomPadding: 10

                    // 動画投稿できないので暫定で読み込めなくする
                    enabled: model.primaryEmbedVideosPaths.length === 0

                    onClicked: {
                        draftListView.currentIndex = model.index
                        console.log("draftListView.currentIndex=" + draftListView.currentIndex)
                    }

                    Rectangle {
                        x: 0 - contentRootFrame.leftPadding
                        y: 0 - contentRootFrame.topPadding
                        width: contentRootFrame.width + contentRootFrame.leftPadding + contentRootFrame.rightPadding
                        height: contentRootFrame.height + contentRootFrame.topPadding + contentRootFrame.bottomPadding
                        color: draftListView.currentIndex === model.index ? Material.highlightedRippleColor : "transparent"
                        radius: 2
                    }

                    ColumnLayout {
                        id: contentRootLayout
                        implicitWidth: basisWidth
                        property real basisWidth: draftScrollView.width -
                                                  contentRootFrame.leftPadding -
                                                  contentRootFrame.rightPadding -
                                                  draftScrollView.ScrollBar.vertical.width

                        RowLayout {
                            Label {
                                Layout.topMargin: 5
                                font.pointSize: AdjustedValues.f8
                                text: model.updatedAt
                            }
                            Item {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1
                            }
                            IconButton {
                                Layout.preferredHeight: AdjustedValues.b26
                                iconSource: "../images/more.png"
                                onClicked: moreMenu.open()
                                MenuEx {
                                    id: moreMenu
                                    Action {
                                        text: qsTr("Discard")
                                        onTriggered: {
                                            discardConfirmationDialog.draftId = model.id
                                            discardConfirmationDialog.show("normal", qsTr("Discard draft?"), qsTr("This draft will be permanently deleted."))
                                        }
                                    }
                                }
                            }

                        }
                        Label {
                            Layout.preferredWidth: parent.width
                            textFormat: Text.StyledText
                            wrapMode: Text.Wrap
                            font.pointSize: AdjustedValues.f10
                            lineHeight: 1.3
                            text: model.primaryText
                        }
                        IconMessage {
                            visible: model.primaryEmbedRecordsUris.length > 0
                            source: "../images/quote.png"
                            text: qsTr("Quote")
                        }
                        RowLayout {
                            id: imagePreviewLayout
                            visible: model.isCurrentDevice &&
                                     model.primaryEmbedImagesPaths.length > 0
                            property var imagePathList: model.primaryEmbedImagesPaths
                            Repeater {
                                model: imagePreviewLayout.imagePathList
                                Image {
                                    Layout.preferredWidth: contentRootLayout.basisWidth / 4 - imagePreviewLayout.spacing * 3
                                    Layout.preferredHeight: Layout.preferredWidth
                                    fillMode: Image.PreserveAspectFit
                                    source: imagePreviewLayout.visible ? model.modelData : ""
                                }
                            }
                        }
                        IconMessage {
                            visible: (model.primaryEmbedVideosPaths.length > 0)
                            source: "../images/warning.png"
                            text: qsTr("Video uploads are not supported.")
                        }
                        IconMessage {
                            visible: !model.isCurrentDevice &&
                                     (model.primaryEmbedImagesPaths.length > 0 ||
                                      model.primaryEmbedVideosPaths.length > 0)
                            source: "../images/warning.png"
                            text: qsTr("Media stored on %s.").replace("%s", model.deviceName)
                        }
                        IconMessage {
                            visible: model.postCount > 1
                            source: "../images/add_circle.png"
                            text: qsTr("%s more posts.").replace("%s", model.postCount - 1)
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
                    selectDraftDialog.reject()
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Apply")
                enabled: draftListView.currentIndex >= 0 && !draftListModel.running
                onClicked: {
                    selectDraftDialog.selectedIndex = draftListView.currentIndex
                    selectDraftDialog.accept()
                }
            }
        }
    }

    MessageDialog {
        id: discardConfirmationDialog
        useCancel: true
        acceptButtonText: qsTr("Discard")
        property string draftId: ""
        onAccepted: {
            if(discardConfirmationDialog.draftId.length > 0){
                console.log("Delete draft:" + discardConfirmationDialog.draftId)
                draftListModel.deleteDraft(discardConfirmationDialog.draftId)
            }
        }
    }
}
