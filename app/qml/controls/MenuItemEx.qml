import QtQuick 2.15
import QtQuick.Controls 2.15

import tech.relog.hagoromo.singleton 1.0

MenuItem {
    icon.width: AdjustedValues.i16
    icon.height: AdjustedValues.i16
    height: AdjustedValues.b36
    font.pointSize: AdjustedValues.f8
    onImplicitWidthChanged: {
        if(menu.width < implicitWidth){
            menu.width = implicitWidth
        }
    }
}
