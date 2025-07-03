import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    width: 840
    height: 250
    visible: true
    title: "Real-time system monitoring"
    property double hRow: 50.0
    property string system_font: Qt.application.font.family
    property double system_size: Qt.application.font.pointSize * 1.0
    property string color1: "#04fc00"
    property string color2: "#e50bc3"

    property var rowsData: [
        { title: system_monitor ? system_monitor.cpu_model_name : "",
            contents: [ system_monitor ? system_monitor.cpu_temperature_c : "",
                system_monitor ? system_monitor.cpu_usage_percent : "",
                system_monitor ? system_monitor.cpu_frequency_mhz : "" ] },

        { title: "Ram",
            contents: [ system_monitor ? system_monitor.memory_usage_percent : "",
                system_monitor ? system_monitor.memory_used_bytes + " Mb of " +
                                 system_monitor.memory_total_bytes + " Mb" : "" ] },

        { title: system_monitor ? system_monitor.gpu_name : "",
            contents: [ system_monitor ? system_monitor.gpu_usage_percent : "",
                system_monitor ? system_monitor.gpu_vram_used + "Mb of " +
                                 system_monitor.gpu_vram_total + "Mb" : "" ] },

        { title: "Network",
            contents: [ system_monitor ? "Down: " + system_monitor.net_rx_bytes + " Kb/s | Up: " +
                                         system_monitor.net_tx_bytes + " Kb/s" : "" ] },

        { title: system_monitor ? system_monitor.disk_model : "",
            contents: [ system_monitor ? "Read: " + system_monitor.disk_read_speed + " Mb/s | Write: " +
                                         system_monitor.disk_write_speed + " Mb/s" : "" ] }
    ]

    Component {
        id: rowComponent
        RowLayout {
            height: root.hRow
            spacing: 2

            Rectangle {
                Layout.preferredWidth: root.width * 1/3
                height: parent.height
                color: "red"
                Label {
                    text: rowTitle
                    font.family: system_font
                    font.pointSize: system_size * 1.2
                    font.bold: true
                    color: color1
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Rectangle {
                Layout.preferredWidth: root.width * 2/3
                height: parent.height
                color: "#202020"
                RowLayout {
                    anchors.fill: parent
                    spacing: 2
                    Repeater {
                        model: rowContents
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            height: parent.height
                            color: "transparent"
                            Label {
                                text: modelData
                                font.family: system_font
                                font.pointSize: system_size
                                font.bold: true
                                color: color2
                                anchors.centerIn: parent
                            }
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        spacing: 2
        anchors.fill: parent

        Repeater {
            model: rowsData
            delegate: Loader {
                property string rowTitle: modelData.title
                property var rowContents: modelData.contents
                sourceComponent: rowComponent
            }
        }
    }
}
