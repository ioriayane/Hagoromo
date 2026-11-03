import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

Flow {
    id: linkLabelLayout
    spacing: 5

    property alias model: repeater.model
    property alias count: repeater.count
    property real fontPointSize: AdjustedValues.f10
    property string hoveredLink: ""

    signal linkActivated(string url)

    Repeater {
        id: repeater
        LinkLabel {
            url: modelData.url
            text: modelData.title
            iconSource: modelData.icon
            providerLogo: modelData.supportProviderLogo
            highlight: modelData.highlight
            fontPointSize: linkLabelLayout.fontPointSize
            onHoveredChanged: {
                if(hovered){
                    linkLabelLayout.hoveredLink = url
                }else if(linkLabelLayout.hoveredLink === url){
                    linkLabelLayout.hoveredLink = ""
                }
            }
            onClicked: (url) => linkLabelLayout.linkActivated(url)
        }
    }
}
