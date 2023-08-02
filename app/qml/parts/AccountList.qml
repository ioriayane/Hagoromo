import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ScrollView {
    id: parts
//    Layout.preferredWidth: 200
//    Layout.preferredHeight: 300
    //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property alias model: accountList.model
    property alias currentIndex: accountList.currentIndex
    signal clicked(int index)

    ListView {
        id: accountList
        delegate: ItemDelegate {
            width: accountList.width
            highlighted: ListView.isCurrentItem
            onClicked: {
                accountList.currentIndex = model.index
                parts.clicked(model.index)
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5
                AvatarImage {
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    source: model.avatar
                }
                Label {
                    text: model.handle
                    elide: Text.ElideRight
                }
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                }
            }
        }
    }
}
