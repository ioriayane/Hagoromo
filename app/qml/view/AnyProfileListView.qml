import QtQuick 2.15

import tech.relog.hagoromo.anyprofilelistmodel 1.0

import "../parts"

ProfileListView {
    id: profileListView

    property alias targetUri: anyProfileListModel.targetUri
    property alias type: anyProfileListModel.type
    property alias autoLoading: anyProfileListModel.autoLoading

    signal errorOccured(string message)

    model: AnyProfileListModel {
        id: anyProfileListModel
        onErrorOccured: (message) => profileListView.errorOccured(message)
    }
}
