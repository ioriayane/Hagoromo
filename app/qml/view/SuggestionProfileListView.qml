import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.searchprofilelistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ListView {
    id: suggestionProfileListView
    property string accountDid: ""

    signal selectedProfile(string did)

    delegate: ClickableFrame {
        id: profileLayout
        clip: true
        style: "Post"
        topPadding: 10
        leftPadding: 10
        rightPadding: 10
        bottomPadding: 10
        enabled: model.associatedChatAllow

        property int layoutWidth: suggestionProfileListView.width
        property var userLabels: model.labels

        onClicked: suggestionProfileListView.selectedProfile(model.did)

        RowLayout{
            AvatarImage {
                id: postAvatarImage
                Layout.preferredWidth: AdjustedValues.i36
                Layout.preferredHeight: AdjustedValues.i36
                Layout.alignment: Qt.AlignTop
                source: model.avatar
                // onClicked: suggestionProfileListView.selectedProfile(model.did)
            }

            ColumnLayout {
                property int basisWidth: profileLayout.layoutWidth - profileLayout.leftPadding - profileLayout.rightPadding -
                                         postAvatarImage.width - parent.spacing
                Label {
                    Layout.preferredWidth: parent.basisWidth
                    font.pointSize: AdjustedValues.f10
                    text: model.displayName
                }
                Label {
                    Layout.preferredWidth: parent.basisWidth
                    font.pointSize: AdjustedValues.f8
                    color: Material.color(Material.Grey)
                    text: model.associatedChatAllow ? ("@" + model.handle) : qsTr("@%s can't be messaged.").replace("%s", model.handle)
                }
                RowLayout {
                    TagLabel {
                        color: Material.color(model.associatedChatAllow ? Material.Green : Material.Red,
                                              Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)
                        source: ""
                        fontPointSize: AdjustedValues.f8
                        text: model.associatedChatAllowIncoming === SearchProfileListModel.AssociatedChatAllowIncomingAll ?
                                  qsTr("Chat:All") : (
                                      model.associatedChatAllowIncoming === SearchProfileListModel.AssociatedChatAllowIncomingFollowing ?
                                          qsTr("Chat:Following") : (
                                              model.associatedChatAllowIncoming === SearchProfileListModel.AssociatedChatAllowIncomingNone ?
                                                  qsTr("Chat:None") : qsTr("Chat:Not set")
                                              )
                                      )
                    }
                    TagLabel {
                        visible: model.following
                        source: ""
                        fontPointSize: AdjustedValues.f8
                        text: qsTr("Following")
                    }
                    TagLabel {
                        visible: model.followedBy
                        source: ""
                        fontPointSize: AdjustedValues.f8
                        text: qsTr("Follows you")
                    }
                    TagLabel {
                        visible: model.muted
                        source: ""
                        fontPointSize: AdjustedValues.f8
                        text: qsTr("Muted user")
                    }
                }
            }
        }
    }
}
