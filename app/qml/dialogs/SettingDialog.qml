import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.encryption 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../compat"
import "../parts"

Dialog {
    id: settingDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    property alias settings: settings

    onRejected: settings.load()

    Encryption {
        id: encryption
    }
    SystemTool {
        id: systemTool
    }

    SettingsC {
        id: settings
        // General
        property int theme: Material.Light
        property color accent: Material.color(Material.Pink)
        property real fontSizeRatio: 1.0
        property string fontFamily: ""
        property real maximumFlickVelocity: 2500
        property real wheelDeceleration: 10000
        property string language: ""
        // Feed
        property string displayOfPosts: "sequential"
        property bool updateSeenNotification: true
        property string realtimeServiceEndpoint: "wss://jetstream1.us-west.bsky.network"
        // Layout
        property int rowCount: 1
        property int rowHeightRatio2: 50
        property int rowHeightRatio31: 35
        property int rowHeightRatio32: 70
        property int imageLayoutType: 1
        // Translate
        property string translateApiUrl: "https://api-free.deepl.com/v2/translate"
        property string translateApiKey: ""
        property string translateTargetLanguage: "JA"
        // About
        property bool displayRealtimeFeedStatus: false
        property bool displayVersionInfoInMainArea: true

        Component.onCompleted: load()

        function load() {
            // Common
            AdjustedValues.ratio = settings.fontSizeRatio
            AdjustedValues.maximumFlickVelocity = settings.maximumFlickVelocity
            // General
            setRadioButton(themeButtonGroup.buttons, settings.theme)
            setRadioButton(accentButtonGroup.buttons, settings.accent)
            fontSizeRatioSlider.value = fontSizeRatio
            setFontFamily(fontFamilyComboBox, settings.fontFamily)
            maximumFlickVelocitySlider.value = settings.maximumFlickVelocity
            wheelDecelerationSlider.value = settings.wheelDeceleration
            languageComboBox.currentIndex = -1
            languageComboBox.setByValue(settings.language)
            // Feed
            setRadioButton(displayOfPostsGroup.buttons, settings.displayOfPosts)
            setRadioButton(updateSeenNotificationGroup.buttons, settings.updateSeenNotification)
            realtimeServiceEndpointComboBox.currentIndex = -1
            realtimeServiceEndpointComboBox.currentIndex = realtimeServiceEndpointComboBox.indexOfValue(settings.realtimeServiceEndpoint)
            // Layout
            rowCountComboBox.currentIndex = -1
            rowCountComboBox.currentIndex = rowCountComboBox.indexOfValue(settings.rowCount)
            rowHeightRatioSlider.value = settings.rowHeightRatio2
            rowHeightRatioRangeSlider.first.value = settings.rowHeightRatio31
            rowHeightRatioRangeSlider.second.value = settings.rowHeightRatio32
            imageLayoutCombobox.setByValue(settings.imageLayoutType)
            // Translate
            translateApiUrlText.text = settings.translateApiUrl
            translateApiKeyText.text = encryption.decrypt(settings.translateApiKey)
            translateTargetLanguageCombo.currentIndex = -1
            translateTargetLanguageCombo.setByValue(settings.translateTargetLanguage)
            // About
            displayRealtimeFeedStatusCheckBox.checked = settings.displayRealtimeFeedStatus
            displayVersionInfoInMainAreaCheckBox.checked = settings.displayVersionInfoInMainArea
        }

        function setRadioButton(buttons, value){
            for(var i=0; i<buttons.length; i++){
                if(buttons[i].value === value){
                    buttons[i].checked = true
                }
            }
        }

        function setFontFamily(combobox, value){
            var ffc_index = combobox.find(value)
            if(ffc_index >= 0){
                combobox.currentIndex = ffc_index
            }else{
                ffc_index = combobox.find(combobox.font.family)
                if(ffc_index >= 0){
                    combobox.currentIndex = ffc_index
                }
            }
        }
    }

    ButtonGroup {
        id: themeButtonGroup
        buttons: themeRowlayout.children
    }
    ButtonGroup {
        id: accentButtonGroup
    }
    ButtonGroup {
        id: displayOfPostsGroup
        buttons: displayOfPostsRowLayout.children
    }
    ButtonGroup {
        id: updateSeenNotificationGroup
        buttons: updateSeenNotificationRowLayout.children
    }

    ColumnLayout {
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            TabButton {
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("General")
            }
            TabButton {
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Feed")
            }
            TabButton {
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Layout")
            }
            TabButton {
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Scroll")
            }
            TabButton {
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Translate")
            }
            TabButton {
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("About")
            }
        }

        SwipeView {
            currentIndex: tabBar.currentIndex
            interactive: false
            clip: true
            implicitWidth: generalPageLayout.width + generalPageFrame.leftPadding + generalPageFrame.rightPadding
            implicitHeight: generalPageLayout.height + generalPageFrame.topPadding + generalPageFrame.bottomPadding

            // General Page
            Frame {
                id: generalPageFrame
                GridLayout {
                    id: generalPageLayout
                    columns: 2
                    rowSpacing: AdjustedValues.s5
                    Label {
                        font.pointSize: AdjustedValues.f10
                        font.family: fontFamilyComboBox.currentText
                        text: qsTr("Theme")
                    }
                    RowLayout {
                        id: themeRowlayout
                        RadioButton {
                            property int value: Material.Light
                            font.pointSize: AdjustedValues.f10
                            font.family: fontFamilyComboBox.currentText
                            text: qsTr("Light")
                        }
                        RadioButton {
                            property int value: Material.Dark
                            font.pointSize: AdjustedValues.f10
                            font.family: fontFamilyComboBox.currentText
                            text: qsTr("Dark")
                        }
                    }
                    Label {
                        font.pointSize: AdjustedValues.f10
                        font.family: fontFamilyComboBox.currentText
                        text: qsTr("Accent color")
                    }
                    GridLayout {
                        id: accentGridLayout
                        columns: 9
                        columnSpacing: 1
                        rowSpacing: 1
                        Repeater {
                            model: [Material.Red, Material.Pink, Material.Purple, Material.DeepPurple,
                                Material.Indigo, Material.Blue, Material.LightBlue, Material.Cyan,
                                Material.Teal, Material.Green, Material.LightGreen, Material.lime,
                                Material.Yellow, Material.Amber, Material.Orange, Material.DeepOrange,
                                Material.Brown, Material.BlueGrey ]
                            Button {
                                id: colorSelectButton
                                Layout.preferredWidth: AdjustedValues.b30
                                Layout.preferredHeight: AdjustedValues.b30
                                topInset: 1
                                leftInset: 1
                                rightInset: 1
                                bottomInset: 1
                                background: Rectangle {
                                    color: Material.color(modelData)
                                    border.color: colorSelectButton.checked ? Material.foreground : Material.background
                                    border.width: 2
                                    radius: 5
                                }
                                checkable: true
                                property color value: Material.color(modelData)
                                ButtonGroup.group: accentButtonGroup
                            }
                        }
                    }

                    Label {
                        Layout.topMargin: AdjustedValues.s10
                        font.pointSize: AdjustedValues.f10
                        font.family: fontFamilyComboBox.currentText
                        text: qsTr("Scaling")
                    }
                    Slider {
                        id: fontSizeRatioSlider
                        Layout.topMargin: AdjustedValues.s10
                        Layout.preferredWidth: 500 * AdjustedValues.ratio
                        from: 0.6
                        to: 2.0
                        stepSize: 0.2
                        snapMode: Slider.SnapOnRelease
                        Label {
                            x: parent.background.x + parent.handle.width / 2 - contentWidth / 2
                            y: parent.topPadding + parent.availableHeight / 2 - parent.handle.height / 2 - contentHeight
                            font.family: fontFamilyComboBox.currentText
                            text: qsTr("A")
                            font.pointSize: AdjustedValues.f10 * parent.from
                        }
                        Label {
                            x: parent.background.x + 2 * parent.width / ((parent.to - parent.from) / parent.stepSize) - contentWidth / 2
                            y: parent.topPadding + parent.availableHeight / 2 - parent.handle.height / 2 - contentHeight
                            font.family: fontFamilyComboBox.currentText
                            text: qsTr("A")
                            font.pointSize: AdjustedValues.f10
                        }
                        Label {
                            id: sliderLabel
                            x: parent.background.x + parent.background.width - parent.handle.width / 2 - contentWidth / 2
                            y: parent.topPadding + parent.availableHeight / 2 - parent.handle.height / 2 - contentHeight
                            font.family: fontFamilyComboBox.currentText
                            text: qsTr("A")
                            font.pointSize: AdjustedValues.f10 * parent.to
                        }
                    }

                    Label {
                        font.pointSize: AdjustedValues.f10
                        font.family: fontFamilyComboBox.currentText
                        text: qsTr("Font family")
                    }
                    ComboBox {
                        id: fontFamilyComboBox
                        Layout.fillWidth: true
                        font.pointSize: AdjustedValues.f10
                        font.family: currentText
                        model: Qt.fontFamilies()
                        delegate: ItemDelegate {
                            width: fontFamilyComboBox.width
                            contentItem: Text {
                                text: modelData
                                color: fontFamilyComboBox.currentIndex === index ? Material.accentColor : Material.foreground
                                //font.family: modelData
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }
                            highlighted: fontFamilyComboBox.highlightedIndex === index
                        }
                    }

                    Label {
                        font.pointSize: AdjustedValues.f10
                        font.family: fontFamilyComboBox.currentText
                        text: qsTr("Language")
                    }
                    ComboBoxEx {
                        id: languageComboBox
                        Layout.fillWidth: true
                        Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                        font.pointSize: AdjustedValues.f10
                        font.family: fontFamilyComboBox.currentText
                        model: ListModel {
                            ListElement { value: "en"; text: qsTr("English") }
                            ListElement { value: "ja"; text: qsTr("Japanese") }
                        }
                    }
                }
            }

            // Feed
            Frame {
                GridLayout {
                    anchors.fill: parent
                    columnSpacing: 5 * AdjustedValues.ratio
                    columns: 2

                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Display of posts")
                    }
                    RowLayout {
                        id: displayOfPostsRowLayout
                        RadioButton {
                            property string value: "sequential"
                            font.pointSize: AdjustedValues.f10
                            text: qsTr("Sequential")
                        }
                        RadioButton {
                            property string value: "at_once"
                            font.pointSize: AdjustedValues.f10
                            text: qsTr("At once")
                        }
                    }

                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Handling notifications")
                    }
                    RowLayout {
                        id: updateSeenNotificationRowLayout
                        RadioButton {
                            property bool value: true
                            font.pointSize: AdjustedValues.f10
                            text: qsTr("Read")
                        }
                        RadioButton {
                            property bool value: false
                            font.pointSize: AdjustedValues.f10
                            text: qsTr("Do nothing")
                        }
                    }

                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Endpoint of Realtime feed")
                    }
                    ComboBox {
                        id: realtimeServiceEndpointComboBox
                        Layout.fillWidth: true
                        Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                        font.pointSize: AdjustedValues.f10
                        model: systemTool.possibleRealtimeFeedServiceEndpoints()
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.columnSpan: 2
                    }
                }
            }

            // Layout
            Frame {
                GridLayout {
                    anchors.fill: parent
                    columnSpacing: 5 * AdjustedValues.ratio
                    columns: 2

                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Row count")
                    }
                    ComboBox {
                        id: rowCountComboBox
                        Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                        font.pointSize: AdjustedValues.f10
                        model: [1, 2, 3]
                    }
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Row height ratio")
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: rowHeightRatioSlider.height
                        RowLayout {
                            Slider {
                                id: rowHeightRatioSlider
                                visible: rowCountComboBox.currentValue !== 3
                                enabled: rowCountComboBox.currentValue === 2
                                from: 0
                                to: 100
                                stepSize: 5
                                snapMode: Slider.SnapAlways
                                onMoved: {
                                    if(value < 10){
                                        value = 10
                                    }else if(value > 90){
                                        value = 90
                                    }
                                }
                            }
                            Label {
                                visible: rowHeightRatioSlider.visible
                                font.pointSize: AdjustedValues.f8
                                text: Math.round(rowHeightRatioSlider.value) + ":"
                                      + Math.round(100 - rowHeightRatioSlider.value)
                            }
                        }
                        RowLayout {
                            RangeSlider {
                                id: rowHeightRatioRangeSlider
                                visible: rowCountComboBox.currentValue === 3
                                from: 0
                                to: 100
                                stepSize: 5
                                snapMode: Slider.SnapAlways
                                first.value: 35
                                second.value: 70
                                first.onMoved: {
                                    if(first.value < 10){
                                        first.value = 10
                                    }else if(first.value > (second.value - 10)){
                                        first.value = second.value - 10
                                    }
                                }
                                second.onMoved: {
                                    if(second.value < (first.value + 10)){
                                        second.value = first.value + 10
                                    }else if(second.value > 90){
                                        second.value = 90
                                    }
                                }
                            }
                            Label {
                                visible: rowHeightRatioRangeSlider.visible
                                font.pointSize: AdjustedValues.f8
                                text: Math.round(rowHeightRatioRangeSlider.first.value) + ":" +
                                      Math.round(rowHeightRatioRangeSlider.second.value - rowHeightRatioRangeSlider.first.value) + ":" +
                                      Math.round(100 - rowHeightRatioRangeSlider.second.value)
                            }
                        }
                    }

                    Label {
                        id: imageLayoutLabel
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Image layout")
                    }
                    ComboBoxEx {
                        id: imageLayoutCombobox
                        Layout.fillWidth: true
                        Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                        model: ListModel {
                            ListElement { value: 0; text: qsTr("Compact") }
                            ListElement { value: 1; text: qsTr("Normal") }
                            ListElement { value: 2; text: qsTr("When one is whole") }
                            ListElement { value: 3; text: qsTr("All whole") }
                        }
                    }
                    Item {
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 1
                    }
                    Label {
                        Layout.topMargin: 0
                        Layout.leftMargin: 2
                        font.pointSize: AdjustedValues.f8
                        text: qsTr("Default value when adding columns.")
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.columnSpan: 2
                    }
                }
            }

            // Scroll
            Frame {
                RowLayout {
                    anchors.fill: parent
                    ColumnLayout {
                        Layout.fillWidth: true

                        RowLayout {
                            Label {
                                font.pointSize: AdjustedValues.f10
                                text: qsTr("Scroll velocity") + ":"
                            }
                            Label {
                                font.pointSize: AdjustedValues.f10
                                text: maximumFlickVelocitySlider.value
                            }
                        }
                        Slider {
                            id: maximumFlickVelocitySlider
                            Layout.fillWidth: true
                            from: 1000
                            to: 5000
                            stepSize: 100
                            snapMode: Slider.SnapOnRelease
                            Rectangle {
                                x: parent.background.x + parent.handle.width / 2 + (parent.background.width - parent.handle.width) * (2500 - parent.from) / (parent.to - parent.from) - width / 2
                                y: parent.topPadding + parent.availableHeight / 2 - parent.handle.height / 2 - height
                                width: 10
                                height: 10
                                radius: 5
                                color: Material.foreground
                            }
                            Label {
                                anchors.left: parent.left
                                anchors.leftMargin: 5
                                anchors.top: parent.top
                                font.pointSize: AdjustedValues.f8
                                text: qsTr("slow")
                            }
                            Label {
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                                anchors.top: parent.top
                                font.pointSize: AdjustedValues.f8
                                text: qsTr("quick")
                            }
                        }

                        RowLayout {
                            Layout.topMargin: AdjustedValues.s5
                            Label {
                                font.pointSize: AdjustedValues.f10
                                text: qsTr("Scroll deceleration") + ":"
                            }
                            Label {
                                font.pointSize: AdjustedValues.f10
                                text: wheelDecelerationSlider.value
                            }
                        }
                        Slider {
                            id: wheelDecelerationSlider
                            Layout.fillWidth: true
                            from: 1000
                            to: 15000
                            stepSize: 1000
                            snapMode: Slider.SnapOnRelease
                            Rectangle {
                                x: parent.background.x + parent.handle.width / 2 + (parent.background.width - parent.handle.width) * (10000 - parent.from) / (parent.to - parent.from) - width / 2
                                y: parent.topPadding + parent.availableHeight / 2 - parent.handle.height / 2 - height
                                width: 10
                                height: 10
                                radius: 5
                                color: Material.foreground
                            }
                            Label {
                                anchors.left: parent.left
                                anchors.leftMargin: 5
                                anchors.top: parent.top
                                font.pointSize: AdjustedValues.f8
                                text: qsTr("slippery")
                            }
                            Label {
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                                anchors.top: parent.top
                                font.pointSize: AdjustedValues.f8
                                text: qsTr("sticky")
                            }
                        }
                        Button {
                            Layout.alignment: Qt.AlignRight
                            font.pointSize: AdjustedValues.f10
                            text: qsTr("Test") + " -> "
                            onClicked: {
                                systemTool.setFlicableWheelDeceleration(wheelDecelerationSlider.value)
                                scrollExampleLoader.sourceComponent = undefined
                                scrollExampleLoader.sourceComponent = scrollExampleComponent
                            }
                            Component {
                                id: scrollExampleComponent
                                ListView {
                                    id: scrollExampleListView
                                    anchors.fill: parent
                                    clip: true
                                    model: ListModel { }
                                    delegate: Frame {
                                        padding: 10
                                        RowLayout {
                                            spacing: 10
                                            AvatarImage {
                                                id: postAvatarImage
                                                Layout.preferredWidth: AdjustedValues.i36
                                                Layout.preferredHeight: AdjustedValues.i36
                                                Layout.alignment: Qt.AlignTop
                                                clip: false
                                            }
                                            ColumnLayout {
                                                property int basisWidth: scrollExampleListView.width -
                                                                         postAvatarImage.width -
                                                                         30
                                                Author {
                                                    id: postAuthor
                                                    Layout.preferredWidth: parent.basisWidth
                                                    layoutWidth: parent.basisWidth
                                                    displayName: model.displayName
                                                    handle: model.handle
                                                }
                                                Label {
                                                    Layout.preferredWidth: parent.basisWidth
                                                    font.pointSize: AdjustedValues.f10
                                                    textFormat: Text.StyledText
                                                    text: model.text
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.leftMargin: AdjustedValues.s5
                            font.pointSize: AdjustedValues.f8
                            wrapMode: Text.Wrap
                            text: qsTr("*) The settings will not be applied until Hagoromo is restarted.")
                        }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 300 * AdjustedValues.ratio
                        Layout.fillHeight: true
                        Label {
                            font.pointSize: AdjustedValues.f10
                            text: "Example:"
                        }
                        Loader {
                            id: scrollExampleLoader
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            onLoaded: {
                                console.log("Loaded")
                                item.maximumFlickVelocity = maximumFlickVelocitySlider.value
                                for(var i=0; i<50; i++){
                                    item.model.append({"displayName": "Name " + i,
                                                          "handle": "Handle " + i,
                                                          "text": "Post " + i + "<br>Post " + i
                                                      })
                                }
                            }
                        }
                    }
                }
            }

            // Translate Page
            Frame {
                GridLayout {
                    anchors.fill: parent
                    columnSpacing: 5 * AdjustedValues.ratio
                    columns: 2
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Api Url")
                    }
                    TextField {
                        id: translateApiUrlText
                        Layout.fillWidth: true
                        font.pointSize: AdjustedValues.f10
                        text: ""
                    }
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Api Key")
                    }
                    TextField {
                        id: translateApiKeyText
                        Layout.fillWidth: true
                        echoMode: TextInput.Password
                        font.pointSize: AdjustedValues.f10
                        text: ""
                    }
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Target language")
                    }
                    ComboBoxEx {
                        id: translateTargetLanguageCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                        model: ListModel {
                            ListElement { value: "BG"; text: qsTr("Bulgarian") }
                            ListElement { value: "ZH"; text: qsTr("Chinese (simplified)") }
                            ListElement { value: "CS"; text: qsTr("Czech") }
                            ListElement { value: "DA"; text: qsTr("Danish") }
                            ListElement { value: "NL"; text: qsTr("Dutch") }
                            ListElement { value: "EN-US"; text: qsTr("English (American)") }
                            ListElement { value: "EN-GB"; text: qsTr("English (British)") }
                            ListElement { value: "ET"; text: qsTr("Estonian") }
                            ListElement { value: "FI"; text: qsTr("Finnish") }
                            ListElement { value: "FR"; text: qsTr("French") }
                            ListElement { value: "DE"; text: qsTr("German") }
                            ListElement { value: "EL"; text: qsTr("Greek") }
                            ListElement { value: "HU"; text: qsTr("Hungarian") }
                            ListElement { value: "ID"; text: qsTr("Indonesian") }
                            ListElement { value: "IT"; text: qsTr("Italian") }
                            ListElement { value: "JA"; text: qsTr("Japanese") }
                            ListElement { value: "KO"; text: qsTr("Korean") }
                            ListElement { value: "LV"; text: qsTr("Latvian") }
                            ListElement { value: "LT"; text: qsTr("Lithuanian") }
                            ListElement { value: "NB"; text: qsTr("Norwegian (Bokmål)") }
                            ListElement { value: "PL"; text: qsTr("Polish") }
                            ListElement { value: "PT-BR"; text: qsTr("Portuguese (Brazilian)") }
                            ListElement { value: "PT-PT"; text: qsTr("Portuguese (excluding Brazilian)") }
                            ListElement { value: "RO"; text: qsTr("Romanian") }
                            ListElement { value: "RU"; text: qsTr("Russian") }
                            ListElement { value: "SK"; text: qsTr("Slovak") }
                            ListElement { value: "SL"; text: qsTr("Slovenian") }
                            ListElement { value: "ES"; text: qsTr("Spanish") }
                            ListElement { value: "SV"; text: qsTr("Swedish") }
                            ListElement { value: "TR"; text: qsTr("Turkish") }
                            ListElement { value: "UK"; text: qsTr("Ukrainian") }
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.columnSpan: 2
                    }
                }
            }

            // About page
            Frame {
                GridLayout {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.margins: 10 * AdjustedValues.ratio
                    columns: 2
                    columnSpacing: 10 * AdjustedValues.ratio
                    rowSpacing: 5 * AdjustedValues.ratio
                    Image {
                        Layout.preferredWidth: AdjustedValues.i50
                        Layout.preferredHeight: AdjustedValues.i50
                        Layout.alignment: Qt.AlignTop
                        Layout.rowSpan: 4
                        source: "../images/logo.png"
                    }
                    Label {
                        font.pointSize: AdjustedValues.f14
                        text: "羽衣 -Hagoromo-"
                    }
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: "Version:" + systemTool.applicationVersion
                    }
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: "build on Qt " + systemTool.qtVersion
                    }
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: "© 2023 Iori Ayane"
                    }
                }
                ColumnLayout {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    CheckBox {
                        id: displayRealtimeFeedStatusCheckBox
                        font.pointSize: AdjustedValues.f8
                        bottomPadding: 1
                        text: qsTr("Display the status of Realtime feed")
                    }
                    CheckBox {
                        id: displayVersionInfoInMainAreaCheckBox
                        font.pointSize: AdjustedValues.f8
                        bottomPadding: 1
                        text: qsTr("Display version info in main area")
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
                    systemTool.setFlicableWheelDeceleration(settings.wheelDeceleration)
                    settingDialog.reject()
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("OK")
                onClicked: {
                    // Common
                    AdjustedValues.ratio = fontSizeRatioSlider.value
                    AdjustedValues.maximumFlickVelocity = maximumFlickVelocitySlider.value
                    systemTool.setFlicableWheelDeceleration(wheelDecelerationSlider.value)
                    // General
                    settings.theme = themeButtonGroup.checkedButton.value
                    settings.accent = accentButtonGroup.checkedButton.value
                    settings.fontSizeRatio = fontSizeRatioSlider.value
                    if(settings.fontFamily !== fontFamilyComboBox.currentText){
                        settings.fontFamily = fontFamilyComboBox.currentText
                        systemTool.updateFont(settings.fontFamily)
                    }
                    settings.maximumFlickVelocity = maximumFlickVelocitySlider.value
                    settings.wheelDeceleration = wheelDecelerationSlider.value
                    settings.language = languageComboBox.currentValue
                    // Feed
                    settings.displayOfPosts = displayOfPostsGroup.checkedButton.value
                    settings.updateSeenNotification = updateSeenNotificationGroup.checkedButton.value
                    settings.realtimeServiceEndpoint = realtimeServiceEndpointComboBox.currentValue
                    // Layout
                    settings.rowCount = rowCountComboBox.currentValue
                    settings.rowHeightRatio2 = rowHeightRatioSlider.value
                    settings.rowHeightRatio31 = rowHeightRatioRangeSlider.first.value
                    settings.rowHeightRatio32 = rowHeightRatioRangeSlider.second.value
                    settings.imageLayoutType = imageLayoutCombobox.currentValue
                    // Translate
                    settings.translateApiUrl = translateApiUrlText.text
                    settings.translateApiKey = encryption.encrypt(translateApiKeyText.text)
                    settings.translateTargetLanguage = translateTargetLanguageCombo.currentValue
                    // About
                    settings.displayRealtimeFeedStatus = displayRealtimeFeedStatusCheckBox.checked
                    settings.displayVersionInfoInMainArea = displayVersionInfoInMainAreaCheckBox.checked

                    settingDialog.accept()
                }
            }
        }
    }
}
