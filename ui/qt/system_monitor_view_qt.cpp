/// MIT License
#include "system_monitor_view_qt.hpp"

namespace ui {
namespace qt {

system_monitor_view_qt::system_monitor_view_qt(presenter::system_monitor_presenter& view_mode,QObject* parent) :
    QObject(parent), view_model_{view_mode} {

    auto bind = [this](auto register_fn, auto signal) {
        (view_model_.*register_fn)([this, signal] {
            QMetaObject::invokeMethod(this, signal, Qt::QueuedConnection);
        });
    };

    bind(&presenter::system_monitor_presenter::on_cpu_changed,    &system_monitor_view_qt::cpu_changed);
    bind(&presenter::system_monitor_presenter::on_memory_changed, &system_monitor_view_qt::memory_changed);
    bind(&presenter::system_monitor_presenter::on_gpu_changed,    &system_monitor_view_qt::gpu_changed);
    bind(&presenter::system_monitor_presenter::on_disk_changed,   &system_monitor_view_qt::disk_changed);
    bind(&presenter::system_monitor_presenter::on_net_changed,    &system_monitor_view_qt::net_changed);
}

QString system_monitor_view_qt::cpu_model_name() const {
    return QString::fromStdString(view_model_.cpu_vm()->model_name);
}

QString system_monitor_view_qt::cpu_usage_percent() const {
    return QString::fromStdString(view_model_.cpu_vm()->usage_percent);
}

QString system_monitor_view_qt::cpu_frequency_mhz() const {
    return QString::fromStdString(view_model_.cpu_vm()->frequency_mhz);
}

QString system_monitor_view_qt::cpu_temperature_c() const {
    return QString::fromStdString(view_model_.cpu_vm()->temperature_c);
}

QString system_monitor_view_qt::cpu_power() const {
    return QString::fromStdString(view_model_.cpu_vm()->power);
}

QString system_monitor_view_qt::memory_vram_used() const {
    return QString::fromStdString(view_model_.memory_vm()->vram_used);
}

QString system_monitor_view_qt::memory_total_bytes() const {
    return QString::fromStdString(view_model_.memory_vm()->vram_total);
}

QString system_monitor_view_qt::memory_used_bytes() const {
    return QString::fromStdString(view_model_.memory_vm()->vram_used);
}

QString system_monitor_view_qt::memory_usage_percent() const {
    return QString::fromStdString(view_model_.memory_vm()->usage_percent);
}

QString system_monitor_view_qt::memory_name() const {
    return QString::fromStdString(view_model_.memory_vm()->name);
}

QString system_monitor_view_qt::memory_voltage() const {
    return QString::fromStdString(view_model_.memory_vm()->voltage);
}

QString system_monitor_view_qt::memory_frequency_mhz() const {
    return QString::fromStdString(view_model_.memory_vm()->frequency_mhz);
}

/// gpu
QString system_monitor_view_qt::gpu_name() const {
    return QString::fromStdString(view_model_.gpu_vm()->name);
}

QString system_monitor_view_qt::gpu_vram_total() const {
    return QString::fromStdString(view_model_.gpu_vm()->vram_total);
}

QString system_monitor_view_qt::gpu_vram_used() const {
    return QString::fromStdString(view_model_.gpu_vm()->vram_used);
}

QString system_monitor_view_qt::gpu_usage_percent() const {
    return QString::fromStdString(view_model_.gpu_vm()->usage_percent);
}

QString system_monitor_view_qt::gpu_cores() const {
    return QString::fromStdString(view_model_.gpu_vm()->cores);
}

QString system_monitor_view_qt::gpu_frequency_mhz() const {
    return QString::fromStdString(view_model_.gpu_vm()->frequency_mhz);
}

QString system_monitor_view_qt::gpu_temperature_c() const {
    return QString::fromStdString(view_model_.gpu_vm()->temperature_c);
}


/// disk
QString system_monitor_view_qt::disk_read_speed() const {
    return QString::fromStdString(view_model_.disk_vm()->read_speed);

}

QString system_monitor_view_qt::disk_write_speed() const {
    return QString::fromStdString(view_model_.disk_vm()->write_speed);
}

QString system_monitor_view_qt::disk_sector_size() const {
    return QString::fromStdString(view_model_.disk_vm()->sector_size);
}

QString system_monitor_view_qt::disk_model() const {
    return QString::fromStdString(view_model_.disk_vm()->model);
}

QString system_monitor_view_qt::disk_serial_number() const {
    return QString::fromStdString(view_model_.disk_vm()->serial_number);
}

QString system_monitor_view_qt::disk_size() const {
    return QString::fromStdString(view_model_.disk_vm()->size);
}

QString system_monitor_view_qt::disk_usage_percent() const {

    return QString::fromStdString(view_model_.disk_vm()->usage_percent);
}

/// net
QString system_monitor_view_qt::net_rx_bytes() const {
    return QString::fromStdString(view_model_.net_vm()->rx_speed);
}

QString system_monitor_view_qt::net_tx_bytes() const {
    return QString::fromStdString(view_model_.net_vm()->tx_speed);
}

} /// namespace qt
} /// namespace ui
