import QtQuick 2.15
import QtQuick.Controls 2.15

import "../controls"

Menu {
    id: tagMenu
    width: tagMenuItem.implicitWidth
    property string tagText: ""
    property bool logMode: false

    signal requestViewSearchPosts(string text)
    signal requestAddMutedWord(string text)

    MenuItemEx {
        enabled: !logMode
        icon.source: "../images/search.png"
        text: qsTr("Search %s posts").replace("%s", tagMenu.tagText)
        onTriggered: requestViewSearchPosts(tagMenu.tagText)
    }
    MenuItemEx {
        id: tagMenuItem
        enabled: !logMode
        icon.source: "../images/account.png"
        text: qsTr("Search %s posts by this user").replace("%s", tagMenu.tagText)
        onTriggered: requestViewSearchPosts(tagMenu.tagText + " from:" + postAuthor.handle)
    }
    MenuSeparator {}
    MenuItemEx {
        icon.source: "../images/mute.png"
        text: qsTr("Mute %s posts").replace("%s", tagMenu.tagText)
        onTriggered: {
            var temp = tagMenu.tagText
            if(temp.charAt(0) === "#"){
                temp = temp.substring(1)
            }
            requestAddMutedWord(temp)
        }
    }
}
