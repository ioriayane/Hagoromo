import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

MouseArea {
    id: externalLinkCardPopup
    anchors.fill: parent

    property int layoutMaximumWidth: 200

    property string linkUri: ""
    property string linkTitle: ""
    property string linkDescription: ""
    property string linkThumb: ""
    property string linkExpiresAt: ""
    property string hoveredLink: ""

    onClicked: {
        if(externalLinkCardPopup.enabled){
            popup.open()
        }
    }

    Popup {
        id: popup
        x: parent.width * 0.9
        y: parent.height * 0.9
        ColumnLayout {
            Label {
                Layout.alignment: Qt.AlignRight
                font.pointSize: AdjustedValues.f10
                text: qsTr("until %1").replace("%1", externalLinkCardPopup.linkExpiresAt)
            }
            ExternalLinkCard {
                width: externalLinkCardPopup.layoutMaximumWidth
                visible: true
                hoverEnabled: true
                uriLabel.text: externalLinkCardPopup.linkUri
                titleLabel.text: externalLinkCardPopup.linkTitle
                descriptionLabel.text: externalLinkCardPopup.linkDescription
                thumbImage.source: externalLinkCardPopup.linkThumb
                onHoveredChanged: {
                    if(hovered){
                        externalLinkCardPopup.hoveredLink = externalLinkCardPopup.linkUri
                    }else{
                        externalLinkCardPopup.hoveredLink = ""
                    }
                }
                onClicked: {
                    Qt.openUrlExternally(externalLinkCardPopup.linkUri)
                    popup.close()
                }
            }
        }
    }
}
