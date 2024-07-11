import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../data"
import "../compat"

Menu {

    delegate: MenuItem {
        id: menuItem
        implicitWidth: 200
        implicitHeight: AdjustedValues.b30
        font.pointSize: AdjustedValues.f8

        property real enabledOpacity: enabled ? 1.0 : 0.5

        arrow: Canvas {
            x: parent.width - width
            implicitWidth: AdjustedValues.b30
            implicitHeight: AdjustedValues.b30
            visible: menuItem.subMenu
            opacity: enabledOpacity
            onPaint: {
                var o = height / 2.5
                var ctx = getContext("2d")
                ctx.fillStyle = menuItem.highlighted ? "#ffffff" : Material.foreground
                ctx.moveTo(o, o)
                ctx.lineTo(width - o, height / 2)
                ctx.lineTo(o, height - o)
                ctx.closePath()
                ctx.fill()
            }
        }

        indicator: Item {
            implicitWidth: AdjustedValues.b30
            implicitHeight: AdjustedValues.b30
            Rectangle {
                width: AdjustedValues.i16
                height: AdjustedValues.i16
                anchors.centerIn: parent
                visible: menuItem.checkable
                border.color: Material.accentColor
                radius: width / 2
                color: "transparent"
                Rectangle {
                    width: AdjustedValues.i10
                    height: AdjustedValues.i10
                    anchors.centerIn: parent
                    visible: menuItem.checked
                    color: Material.accentColor
                    radius: width / 2
                }
            }
            Image {
                width: AdjustedValues.i16
                height: AdjustedValues.i16
                anchors.centerIn: parent
                visible: !menuItem.checkable
                source: menuItem.icon.source
                layer.enabled: true
                layer.effect: ColorOverlayC {
                    color: Material.foreground
                }
            }
        }

        contentItem: Label {
            leftPadding: 14 + 22 * (AdjustedValues.ratio - 1)
            rightPadding: menuItem.arrow.implicitWidth * 2
            text: menuItem.text
            font: menuItem.font
            opacity: enabledOpacity
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            onContentWidthChanged: {
                if(!menuItem.menu){
                    return
                }
                var w = leftPadding + rightPadding + contentWidth
                if(menuItem.menu.width < w){
                    // console.log("update width " + menuItem.menu.width + "->" + w + "(" + text + ")")
                    menuItem.implicitWidth = w
                    menuItem.menu.width = w
                }
            }
        }

        background: Rectangle {
            opacity: enabledOpacity
            color: menuItem.highlighted ?
                       Material.color(Material.Grey, Material.theme === Material.Dark ?
                                          Material.Shade600 : Material.Shade400)
                     : "transparent"
        }
    }

    background: Rectangle {
        color: Material.backgroundColor
        border.color: Material.color(Material.Grey, Material.theme === Material.Dark ?
                                         Material.Shade600 : Material.Shade400)
        border.width: 1
        radius: 2
    }
}
