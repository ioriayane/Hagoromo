import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.anyprofilelistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: anyProfileListView

    property alias hoveredLink: profileListView.hoveredLink
    property alias accountDid: profileListView.accountDid   // 取得するユーザー

    property alias targetUri: anyProfileListModel.targetUri
    property alias type: anyProfileListModel.type
    property alias autoLoading: anyProfileListModel.autoLoading
    property alias model: anyProfileListModel

    signal requestViewProfile(string did)
    signal errorOccured(string code, string message)
    signal back()

    Frame {
        Layout.fillWidth: true
        leftPadding: 0
        topPadding: 0
        rightPadding: 10
        bottomPadding: 0

        RowLayout {
            IconButton {
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b30
                flat: true
                iconSource: "../images/arrow_left_single.png"
                onClicked: anyProfileListView.back()
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 10
                font.pointSize: AdjustedValues.f10
                text: anyProfileListModel.type == AnyProfileListModel.Like ? qsTr("Liked by") : qsTr("Reposted by")
            }
        }
    }

    ProfileListView {
        id: profileListView
        Layout.fillWidth: true
        Layout.fillHeight: true
        unfollowAndRemove: false

        model: AnyProfileListModel {
            id: anyProfileListModel
            onErrorOccured: (code, message) => anyProfileListView.errorOccured(code, message)
        }

        onRequestViewProfile: (did) => anyProfileListView.requestViewProfile(did)
    }
}
