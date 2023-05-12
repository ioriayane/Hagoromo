import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

ProgressBar {
    id: control

    contentItem: Item{}
    background: Item {
        implicitWidth: 50
        implicitHeight: 50

        readonly property real size: Math.min(control.width, control.height)
        width: size
        height: size
        anchors.centerIn: parent

        Canvas {
            id: canvas
            anchors.fill: parent

            Connections {
                target: control
                function onPositionChanged() { canvas.requestPaint(); }
            }

            onPaint: {
                var basis_line_width = parent.size / 8
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.strokeStyle = Material.accentColor
                ctx.lineWidth = basis_line_width
                ctx.beginPath()
                var startAngle = -Math.PI / 2
                var endAngle = startAngle + control.position * 2 * Math.PI
                ctx.arc(width / 2, height / 2, width / 2 - basis_line_width / 2 - 2, startAngle, endAngle)
                ctx.stroke()

                ctx.strokeStyle = Material.color(Material.Grey)
                ctx.lineWidth = basis_line_width / 3
                ctx.beginPath()
                ctx.arc(width / 2, height / 2, width / 2 - basis_line_width / 2 - 2, endAngle, startAngle + 2 * Math.PI)
                ctx.stroke()
            }
        }
    }
}
