import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

import "controls" as C
import org.streetpea.chiaking

DialogView {
    id: dialog
    title: qsTr("Custom Scripts")
    header: qsTr("JavaScript automation")
    buttonText: qsTr("Done")
    buttonVisible: true
    onAccepted: close()

    FileDialog {
        id: importDialog
        title: qsTr("Import a JavaScript script")
        nameFilters: [qsTr("JavaScript files (*.js)")]
        onAccepted: {
            Chiaki.scripts.addScriptFromFile(selectedFile);
        }
    }

    ColumnLayout {
        spacing: 18
        anchors.margins: 30

        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: qsTr("Scripts run locally inside a restricted JavaScript engine. They can log, show notifications, and react to app and session events, but they do not receive shell, filesystem, or network APIs.")
            color: Material.secondaryTextColor
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            TextField {
                id: newScriptName
                Layout.fillWidth: true
                placeholderText: qsTr("New script name")
                onAccepted: createButton.clicked()
            }
            Button {
                id: createButton
                text: qsTr("Create")
                onClicked: {
                    if (Chiaki.scripts.createScript(newScriptName.text))
                        newScriptName.clear();
                }
            }
            Button {
                text: qsTr("Import")
                onClicked: importDialog.open()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                Layout.fillWidth: true
                text: qsTr("Folder: %1").arg(Chiaki.scripts.scriptsDirectory)
                elide: Text.ElideMiddle
                color: Material.secondaryTextColor
                font.pixelSize: 12
            }
            Button {
                text: qsTr("Open Folder")
                onClicked: Chiaki.scripts.openScriptsFolder()
            }
        }

        ListView {
            id: scriptList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: Chiaki.scripts.scripts
            delegate: Frame {
                required property string fileName
                required property string name
                required property bool enabled
                required property string error
                width: scriptList.width
                padding: 12
                RowLayout {
                    anchors.fill: parent
                    CheckBox {
                        checked: enabled
                        onToggled: Chiaki.scripts.setScriptEnabled(fileName, checked)
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: name; font.bold: true }
                        Label {
                            Layout.fillWidth: true
                            text: error.length ? error : qsTr("Ready")
                            color: error.length ? Material.accentColor : Material.secondaryTextColor
                            elide: Text.ElideRight
                            font.pixelSize: 12
                        }
                    }
                    Button {
                        text: qsTr("Run")
                        enabled: !error.length
                        onClicked: Chiaki.scripts.runScript(fileName)
                    }
                    Button {
                        text: qsTr("Delete")
                        onClicked: Chiaki.scripts.removeScript(fileName)
                    }
                }
            }
            Label {
                anchors.centerIn: parent
                visible: scriptList.count === 0
                text: qsTr("No custom scripts yet")
                color: Material.secondaryTextColor
            }
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: qsTr("Hooks: onAppStart, onBeforeConnect, onSessionStarted, onBeforeDisconnect. Each receives (api, event). See the README for examples.")
            color: Material.secondaryTextColor
            font.pixelSize: 12
        }
    }
}
