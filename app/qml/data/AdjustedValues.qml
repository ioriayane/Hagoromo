pragma Singleton
import QtQuick 2.15

QtObject {
    property real ratio: 1.0
    property real ratioHalf: 1.0 + (ratio - 1.0) / 2

    // for Button
    property real b55: 55 * ratio
    property real b36: 36 * ratio
    property real b30: 30 * ratio
    property real b26: 26 * ratio
    property real b24: 24 * ratio

    // for Icon
    property real i50: 50 * ratio
    property real i48: 48 * ratio
    property real i36: 36 * ratio
    property real i32: 32 * ratio
    property real i24: 24 * ratio
    property real i20: 20 * ratio
    property real i18: 18 * ratio
    property real i16: 16 * ratio
    property real i12: 12 * ratio
    property real i10: 10 * ratio

    // for Font
    property real f14: 14 * ratio
    property real f12: 12 * ratio
    property real f10: 10 * ratio
    property real f8: 8 * ratio
    property real f6: 6 * ratio

    // for header
    property real h32: 32 * ratio
    property real h24: 24 * ratio

    // for spacing
    property real s20: 20 * ratio
    property real s15: 15 * ratio
    property real s10: 10 * ratio
    property real s5: 5 * ratio

    // others
    property real v96: 96 * ratio
    property real v72: 72 * ratio
    property real v48: 48 * ratio
    property real v24: 24 * ratio

    property real maximumFlickVelocity: 2500
}
