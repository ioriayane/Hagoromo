pragma Singleton
import QtQuick 2.15

QtObject {
    property real ratio: 1.0
    property real ratioHalf: 1.0 + (ratio - 1.0) / 2

    // for Button
    property int b55: 66 * ratio
    property int b36: 42 * ratio
    property int b30: 36 * ratio
    property int b26: 32 * ratio
    property int b24: 28 * ratio

    // for Icon
    property int i60: 72 * ratio
    property int i50: 60 * ratio
    property int i48: 56 * ratio
    property int i36: 42 * ratio
    property int i32: 38 * ratio
    property int i24: 28 * ratio
    property int i20: 24 * ratio
    property int i18: 22 * ratio
    property int i16: 20 * ratio
    property int i12: 14 * ratio
    property int i10: 12 * ratio

    // for Font
    property int f14: 16 * ratio
    property int f12: 14 * ratio
    property int f10: 12 * ratio
    property int f8: 10 * ratio
    property int f6: 8 * ratio

    // for header
    property int h32: 38 * ratio
    property int h24: 28 * ratio

    // for spacing
    property int s20: 24 * ratio
    property int s15: 18 * ratio
    property int s10: 12 * ratio
    property int s5: 6 * ratio

    // others
    property int v96: 116 * ratio
    property int v72: 86 * ratio
    property int v48: 58 * ratio
    property int v24: 28 * ratio

    property real maximumFlickVelocity: 2500
}
