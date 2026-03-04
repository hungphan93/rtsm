/// MIT License
#include "system_monitor_qt.hpp"

namespace ui {
namespace qt {

system_monitor_qt::system_monitor_qt(presenter::system_monitor_view_model& view_mode,QObject* parent) :
    QObject(parent), view_model_{view_mode} {

    auto bind = [this](auto register_fn, auto signal) {
        (view_model_.*register_fn)([this, signal] {
            QMetaObject::invokeMethod(this, signal, Qt::QueuedConnection);
        });
    };

    bind(&presenter::system_monitor_view_model::on_cpu_changed,    &system_monitor_qt::cpu_changed);
    bind(&presenter::system_monitor_view_model::on_memory_changed, &system_monitor_qt::memory_changed);
    bind(&presenter::system_monitor_view_model::on_gpu_changed,    &system_monitor_qt::gpu_changed);
    bind(&presenter::system_monitor_view_model::on_disk_changed,   &system_monitor_qt::disk_changed);
    bind(&presenter::system_monitor_view_model::on_net_changed,    &system_monitor_qt::net_changed);
}

QString system_monitor_qt::cpu_model_name() const {
    return QString::fromStdString(view_model_.cpu().model_name);
}

QString system_monitor_qt::cpu_usage_percent() const {
    return QString::number(view_model_.cpu().usage_percent, 'f', 2) + "%";
}

QString system_monitor_qt::cpu_frequency_mhz() const {
    return QString::number(view_model_.cpu().frequency_mhz, 'f', 0) + "Mhz";
}

QString system_monitor_qt::cpu_temperature_c() const {
    return QString::number(view_model_.cpu().temperature_c, 'f', 0) + "°C";
}

QString system_monitor_qt::cpu_power() const {
    return QString::number(view_model_.cpu().power_uw / 1000000, 'f', 1) + "W";
}

QString system_monitor_qt::memory_vram_used() const {
    return QString::number(view_model_.memory().vram_used);
}

QString system_monitor_qt::memory_total_bytes() const {
    return QString::number(view_model_.memory().vram_total, 'f', 1);
}

QString system_monitor_qt::memory_used_bytes() const {
    return QString::number(view_model_.memory().vram_used, 'f', 1);
}

QString system_monitor_qt::memory_usage_percent() const {
    return QString::number(view_model_.memory().usage_percent, 'f', 2) + "%";
}

QString system_monitor_qt::memory_name() const {
    return QString::fromStdString(view_model_.memory().name);
}

QString system_monitor_qt::memory_voltage() const {
    return QString::number(view_model_.memory().voltage, 'f', 1) + "V";
}

QString system_monitor_qt::memory_frequency_mhz() const {
    return QString::number(view_model_.memory().frequency_mhz, 'f', 0) + "Mhz";
}

/// gpu
QString system_monitor_qt::gpu_name() const {
    return QString::fromStdString(view_model_.gpu().name);
}

QString system_monitor_qt::gpu_vram_total() const {
    return QString::number(view_model_.gpu().vram_total);
}

QString system_monitor_qt::gpu_vram_used() const {
    return QString::number(view_model_.gpu().vram_used);
}

QString system_monitor_qt::gpu_usage_percent() const {
    return QString::number(view_model_.gpu().usage_percent) + "%";
}

QString system_monitor_qt::gpu_cores() const {
    return QString::number(view_model_.gpu().cores);
}

QString system_monitor_qt::gpu_frequency_mhz() const {
    return QString::number(view_model_.gpu().frequency_mhz) + "Mhz";
}

QString system_monitor_qt::gpu_temperature_c() const {
    return QString::number(view_model_.gpu().temperature_c) + "°C";
}

QString format_speed_fast(double data) {
    /// compile time
    constexpr double kb = 1024.0;
    constexpr double mb = kb * 1024.0;
    constexpr double gb = mb * 1024.0;

    if (data < kb) {
        return QString::number(data, 'f', 1) + " Bytes/s";
    }
    else if (data < mb) {
        return QString::number(data / kb, 'f', 1) + " KiB/s";
    }
    else if (data < gb) {
        return QString::number(data / mb, 'f', 1) + " MiB/s";
    }
    else {
        return QString::number(data / gb, 'f', 2) + " GiB/s";
    }
}

/// disk
QString system_monitor_qt::disk_read_speed() const {
    return format_speed_fast(view_model_.disk().read_speed);

}

QString system_monitor_qt::disk_write_speed() const {
    return format_speed_fast(view_model_.disk().write_speed);
}

QString system_monitor_qt::disk_sector_size() const {
    return QString::number(view_model_.disk().sector_size);
}

QString system_monitor_qt::disk_model() const {
    return QString::fromStdString(view_model_.disk().model);
}

QString system_monitor_qt::disk_serial_number() const {
    return QString::fromStdString(view_model_.disk().serial_number);
}

QString system_monitor_qt::disk_size() const {
    return QString::number(view_model_.disk().size);
}

QString system_monitor_qt::disk_usage_percent() const {
    auto total = view_model_.disk().total;
    return QString::number(total > 0 ? 100.0 * view_model_.disk().used / total : 0.0, 'f', 2) + "%";
}

/// net
QString system_monitor_qt::net_rx_bytes() const {
    return format_speed_fast(view_model_.net().rx_bytes);
}

QString system_monitor_qt::net_tx_bytes() const {
    return format_speed_fast(view_model_.net().tx_bytes);
}

} /// namespace qt
} /// namespace ui
