import QtQuick
import QtQuick.Controls

Window {
    id: root
    width: 840
    height: 200
    visible: true
    property double hRow: 50
    Column {
        spacing: 2
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        Row {
            id: groupCPU
            anchors.top: parent.top
            height: root.hRow
            anchors.right: parent.right
            anchors.left: parent.left
            Rectangle {
                width: parent.width / 4 + 90
                height: parent.height
                Label {
                    text: systemMonitor.cpuModelName
                    font.pixelSize: 22
                    font.bold: true
                    color: "#04fc00"
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: parent.height
                Label {
                    text: systemMonitor.cpuTemperatureC/1000 + " °C"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: parent.height
                Label {
                    text: systemMonitor.cpuUsagePercent + " %"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: parent.height
                Label {
                    text: systemMonitor.cpuFrequencyMhz + " MHZ"
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
            height: root.hRow
            anchors.right: parent.right
            anchors.left: parent.left
            Rectangle {
                width: parent.width / 4 + 90
                height: parent.height
                Label {
                    text: "Ram"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#04fc00"
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: parent.height
                Label {
                    text: " " + systemMonitor.memoryPercent + " %"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 2 - 100
                height: parent.height
                Label {
                    text: systemMonitor.memoryUsed + " MB of " +  systemMonitor.memoryTotal + " MB"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }

        Row {
            id: groupGpu
            spacing: 2
            anchors.top: groupMemory.bottom
            height: root.hRow
            anchors.right: parent.right
            anchors.left: parent.left
            Rectangle {
                width: parent.width / 4 + 90
                height: parent.height
                Label {
                    text: systemMonitor.gpuName
                    font.pixelSize: 22
                    font.bold: true
                    color: "#04fc00"
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: parent.height
                Label {
                    text: "   " + systemMonitor.gpuVramTotal + " MB"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 4 - 30
                height: parent.height
                Label {
                    text: systemMonitor.gpuVramUsed + " MB"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }

        Row {
            id: groupNet
            spacing: 2
            anchors.top: groupGpu.bottom
            height: root.hRow
            anchors.right: parent.right
            anchors.left: parent.left
            Rectangle {
                width: parent.width / 4 + 90
                height: parent.height
                Label {
                    text: "Network"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#04fc00"
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                width: parent.width / 4 - 50
                height: parent.height
                Label {
                    text: "Download: " + systemMonitor.netRxBytes + " KB/s | "
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: parent.width / 4 - 30
                height: parent.height
                Label {
                    text: "Upload: " + systemMonitor.netTxBytes + " KB/s"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }
    }
}
