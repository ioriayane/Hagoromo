pragma Singleton
import QtQuick 2.15

QtObject {
    property real ratio: 1.0
    property real ratioHalf: 1.0 + (ratio - 1.0) / 2

    // for Button
    property real b55: 66 * ratio
    property real b36: 42 * ratio
    property real b30: 36 * ratio
    property real b26: 32 * ratio
    property real b24: 28 * ratio

    // for Icon
    property real i50: 60 * ratio
    property real i48: 56 * ratio
    property real i36: 42 * ratio
    property real i32: 38 * ratio
    property real i24: 28 * ratio
    property real i20: 24 * ratio
    property real i18: 22 * ratio
    property real i16: 20 * ratio
    property real i12: 14 * ratio
    property real i10: 12 * ratio

    // for Font
    property real f14: 16 * ratio
    property real f12: 14 * ratio
    property real f10: 12 * ratio
    property real f8: 10 * ratio
    property real f6: 8 * ratio

    // for header
    property real h32: 38 * ratio
    property real h24: 28 * ratio

    // for spacing
    property real s20: 24 * ratio
    property real s15: 18 * ratio
    property real s10: 12 * ratio
    property real s5: 6 * ratio

    // others
    property real v96: 116 * ratio
    property real v72: 86 * ratio
    property real v48: 58 * ratio
    property real v24: 28 * ratio

    property real maximumFlickVelocity: 2500
}
