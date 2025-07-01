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
            id: group_cpu
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
                    text: system_monitor ? system_monitor.cpu_temperature_c : ""
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
                    text: system_monitor ? system_monitor.cpu_usage_percent: ""
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
                    text: system_monitor ? system_monitor.cpu_frequency_mhz : ""
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }



        Row {
            id: group_memory
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
                    text: system_monitor ? system_monitor.memory_vram_used : ""
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
                    text: system_monitor ? system_monitor.memory_used_bytes + " MB of " +  system_monitor.memory_total_bytes + " MB" : ""
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }

        Row {
            id: group_gpu
            spacing: 2
            height: root.hRow
            width: parent.width
            Rectangle {
                width: parent.width / 4 + 90
                height: parent.height
                Label {
                    text: system_monitor ? system_monitor.gpu_name : ""
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
                    text: system_monitor ? system_monitor.gpu_vram_total + " MB": ""
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
                    text: system_monitor ? system_monitor.gpu_vram_used + " MB" : ""
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }

        Row {
            id: group_net
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
                    text:  system_monitor ? "Download: " + system_monitor.net_rx_bytes + " KB/s | " : ""
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
                    text:  system_monitor ? "Upload: " + system_monitor.net_tx_bytes + " KB/s" : ""
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }

        Row {
            id: group_disk
            spacing: 2
            height: root.hRow
            width: parent.width
            Rectangle {
                width: parent.width / 4 + 90
                height: parent.height
                Label {
                    text: system_monitor ? system_monitor.disk_model : ""
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
                    text: system_monitor ? "Read: "  + system_monitor.disk_read_speed + " MB/s | " : ""
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
                    text: system_monitor ? "Write: " + system_monitor.disk_write_speed + " MB/s" : ""
                    font.pixelSize: 22
                    font.bold: true
                    color: "#e50bc3"
                    anchors.centerIn: parent
                }
            }
        }
    }
}
