/// MIT License
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    width: 1200
    height: 250
    visible: true
    /// flags: Qt.SplashScreen | Qt.FramelessWindowHint
    flags: Qt.SplashScreen | Qt.FramelessWindowHint | Qt.WindowTransparentForInput /// to click folder below the app
    color: "transparent"
    title: "Real-time system monitoring"
    x: 40
    y: 30
    readonly property font system_font: ({
                                             family: "Ubuntu Sans",
                                             pointSize: 14,
                                             weight: Font.ExtraBold
                                         })

    property string color_device_name: "#00a107"
    property string color_device_detail: "#e600d7"

    property var rowsData: [
        {
            title: system_monitor ? system_monitor.cpu_model_name : "",
            contents:
                [
                system_monitor ? system_monitor.cpu_usage_percent : "",
                system_monitor ? system_monitor.cpu_temperature_c : "",
                system_monitor ? system_monitor.cpu_power_mw : "",
                system_monitor ? system_monitor.cpu_frequency_mhz : ""
            ]
        },

        {
            title: system_monitor ? system_monitor.memory_name : "Ram",
            contents:
                [
                system_monitor ? system_monitor.memory_usage_percent : "",
                system_monitor ? system_monitor.memory_power_mw : "",
                system_monitor ? system_monitor.memory_used_bytes + "Gb of " +
                                 system_monitor.memory_total_bytes + "Gb" : "",
                system_monitor ? system_monitor.memory_frequency_mhz : "",
            ]
        },

        {
            title: system_monitor ? system_monitor.gpu_name : "",
            contents:
                [
                system_monitor ? system_monitor.gpu_usage_percent : "",
                system_monitor ? system_monitor.gpu_temperature_c : "",
                system_monitor ? system_monitor.gpu_vram_used + "Mb of " +
                                 system_monitor.gpu_vram_total + "Mb" : "",
                system_monitor ? system_monitor.gpu_frequency_mhz : "",
            ]
        },

        {
            title: system_monitor ? system_monitor.disk_model : "",
            contents:
                [
                system_monitor ? system_monitor.disk_usage_percent : "",
                system_monitor ? "Read: " + system_monitor.disk_read_speed + "Mb/s | Write: " +
                                 system_monitor.disk_write_speed + "Mb/s" : ""
            ]
        },

        {
            title: "Network",
            contents:
                [
                system_monitor ? "Down: " + system_monitor.net_rx_bytes + "Kb/s | Up: " +
                                 system_monitor.net_tx_bytes + "Kb/s" : ""
            ]
        },


    ]

    Component {
        id: rowComponent
        RowLayout {
            spacing: 2
            Rectangle {
                Layout.preferredWidth: root.width * 1/3
                height: parent.height
                // color: "red"
                Label {
                    text: row_title
                    font: system_font
                    elide: Text.ElideRight
                    clip: true
                    color: color_device_name
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Rectangle {
                Layout.preferredWidth: root.width * 2/3
                height: parent.height
                RowLayout {
                    anchors.fill: parent
                    spacing: 2
                    Repeater {
                        model: 4
                        delegate: Rectangle {
                            Layout.preferredWidth: (root.width * 2/3) / 4
                            height: parent.height
                            // color: "transparent"
                            Label {
                                text: index < row_contents.length ? row_contents[index] : ""
                                font: system_font
                                color: color_device_detail
                                anchors.left: parent.left
                                anchors.leftMargin: 5
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        spacing: 1
        anchors.fill: parent
        Repeater {
            model: rowsData
            delegate: Loader {
                property string row_title: modelData.title
                property var row_contents: modelData.contents
                sourceComponent: rowComponent
            }
        }
    }
}
