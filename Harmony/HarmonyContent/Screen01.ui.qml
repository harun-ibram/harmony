/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Controls
import Harmony

Rectangle {
    id: rectangle
    width: Constants.width
    height: Constants.height

    color: Constants.backgroundColor

    Column {
        id: column
        x: 0
        y: 0
        width: 1920
        height: 1080
        visible: true

        Rectangle {
            id: rectangle1
            width: 1767
            height: 712
            color: "#1d2336"

            TextEdit {
                id: textEdit
                x: 28
                y: 72
                width: 1693
                height: 538
                text: qsTr("Text Edit")
                font.pixelSize: 12

                SwipeDelegate {
                    id: swipeDelegate
                    x: 0
                    y: 0
                    width: 160
                    height: 477
                    text: qsTr("Swipe Delegate")

                    TextField {
                        id: textField
                        x: 23
                        y: 14
                        placeholderText: qsTr("Text Field")
                    }
                }
            }
        }
    }
    states: [
        State {
            name: "clicked"
        }
    ]
}
