import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15
import tech.relog.hagoromo.singleton 1.0

TabBar {
    id: tabBar

    property string profileSource: ""

    TabButton {
        topPadding: 0
        bottomPadding: 0
        contentItem: Item {
            Image {
                width: AdjustedValues.i16
                height: AdjustedValues.i16
                anchors.centerIn: parent
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.foreground
                }
                source: "../images/reply.png"
            }
        }
//        background: Item {
//            implicitHeight: 30
//        }
    }
    TabButton {
        topPadding: 0
        bottomPadding: 0
        contentItem: Item {
            Image {
                width: AdjustedValues.i16
                height: AdjustedValues.i16
                anchors.centerIn: parent
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.color(Material.Green)
                }
                source: "../images/repost.png"
            }
        }
//        background: Item {
//            implicitHeight: 30
//        }
    }
    TabButton {
        topPadding: 0
        bottomPadding: 0
        contentItem: Item {
            Image {
                width: AdjustedValues.i16
                height: AdjustedValues.i16
                anchors.centerIn: parent
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.color(Material.Pink)
                }
                source: "../images/like.png"
            }
        }
//        background: Item {
//            implicitHeight: 30
//        }
    }
    TabButton {
        topPadding: 0
        bottomPadding: 0
        contentItem: Item {
            Image {
                width: AdjustedValues.i16
                height: AdjustedValues.i16
                anchors.centerIn: parent
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.color(Material.Blue)
                }
                source: "../images/media.png"
            }
        }
//        background: Item {
//            implicitHeight: 30
//        }
    }
    TabButton {
        topPadding: 0
        bottomPadding: 0
        leftPadding: 0
        rightPadding: 0
        contentItem: RowLayout {
            spacing: 1
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Item {
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                Image {
                    id: avatorImage1
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    source: tabBar.profileSource
                    visible: false
                }
                Image {
                    id: mask1
                    anchors.fill: parent
                    source: "../images/icon_mask.png"
                    visible: false
                }
                OpacityMask {
                    anchors.fill: parent
                    source: avatorImage1
                    maskSource: mask1
                }
            }
            Image {
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.foreground
                }
                source: "../images/arrow_forward.png"
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
        }
//        background: Item {
//            implicitHeight: 30
//        }
    }
    TabButton {
        topPadding: 0
        bottomPadding: 0
        leftPadding: 0
        rightPadding: 0
        contentItem: RowLayout {
            spacing: 1
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Item {
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                Image {
                    id: avatorImage2
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    source: tabBar.profileSource
                    visible: false
                }
                Image {
                    id: mask2
                    anchors.fill: parent
                    source: "../images/icon_mask.png"
                    visible: false
                }
                OpacityMask {
                    anchors.fill: parent
                    source: avatorImage2
                    maskSource: mask2
                }
            }
            Image {
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.foreground
                }
                source: "../images/arrow_back.png"
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
        }
//        background: Item {
//            implicitHeight: 30
//        }
    }
}
