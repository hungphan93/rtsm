import QtQuick
import QtQuick.Controls

Window {
    id: root
    width: 840
    height: 250
    visible: true
    property double hRow: 50.0
    Column {
        spacing: 2
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        Row {
            id: groupCPU
            height: root.hRow
            width: parent.width
            Rectangle {
                width: parent.width / 4 + 90
                height: parent.height
                Label {
                    text: system_monitor ? system_monitor.cpu_model_name : ""
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
                    text: system_monitor ? system_monitor.cpu_temperature_c/1000 + " °C" : ""
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
                    text: system_monitor ? system_monitor.cpu_usage_percent + " %" : ""
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
                    text: system_monitor.cpu_frequency_mhz
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
            height: root.hRow
            width: parent.width
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
                    text: systemMonitor ? systemMonitor.memoryPercent + " %" : ""
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
                    text: systemMonitor ? systemMonitor.memoryUsed + " MB of " +  systemMonitor.memoryTotal + " MB" : ""
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
            height: root.hRow
            width: parent.width
            Rectangle {
                width: parent.width / 4 + 90
                height: parent.height
                Label {
                    text: systemMonitor ? systemMonitor.gpuName : ""
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
                    text: systemMonitor ? systemMonitor.gpuVramTotal + " MB": ""
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
                    text: systemMonitor ? systemMonitor.gpuVramUsed + " MB" : ""
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
            height: root.hRow
            width: parent.width
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
                    text:  systemMonitor ? "Download: " + systemMonitor.netRxBytes + " KB/s | " : ""
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
                    text:  systemMonitor ? "Upload: " + systemMonitor.netTxBytes + " KB/s" : ""
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }

        Row {
            id: groupDisk
            spacing: 2
            height: root.hRow
            width: parent.width
            Rectangle {
                width: parent.width / 4 + 90
                height: parent.height
                Label {
                    text: systemMonitor ? systemMonitor.diskModel : ""
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
                    text: systemMonitor ? "Read: "  + systemMonitor.diskReadSpeed + " MB/s | " : ""
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
                    text: systemMonitor ? "Write: " + systemMonitor.diskWriteSpeed + " MB/s" : ""
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }
    }
}
