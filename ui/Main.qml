import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    Column {
        spacing: 2
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        Row {
            id: groupCPU
            anchors.top: parent.top
            height: 50
            width: parent.width
            Rectangle {
                width: parent.width / 4 + 148
                height: 50
                Label {
                    text: systemMonitor.modelName
                    font.pixelSize: 22
                    font.bold: true
                    color: "#04fc00"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: 50
                Label {
                    text: systemMonitor.temperatureC/1000 + " °C"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: 50
                Label {
                    text: systemMonitor.usagePercent
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: 50
                Label {
                    text: systemMonitor.frequencyMhz + " MHZ"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }



        Row {
            id: groupMemory
            spacing: 2
            anchors.top: groupCPU.bottom
            height: 50
            width: parent.width
            Rectangle {
                width: parent.width / 4 + 148
                height: 50
                Label {
                    text: "Memory"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#04fc00"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: 50
                Label {
                    text: " " + systemMonitor.memoryPercent
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: 50
                Label {
                    text: systemMonitor.memoryUsed + "MB/" +  systemMonitor.memoryTotal + "MB"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }
    }
}
