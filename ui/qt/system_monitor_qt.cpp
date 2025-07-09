#include "system_monitor_qt.hpp"
#include <QTimer>

namespace ui {
namespace qt {

system_monitor_qt::system_monitor_qt(presenter::system_monitor* presenter,QObject* parent) :
    QObject(parent), presenter_(presenter) {
    connect(&timer_, &QTimer::timeout, this, &system_monitor_qt::refresh_ui);
    timer_.start(100);
}

QString system_monitor_qt::cpu_model_name() const {
    if (!presenter_) return {};

    const auto cpu = presenter_->cpu();
    return QString::fromStdString(cpu.model_name);
}

QString system_monitor_qt::cpu_usage_percent() const {
    if (!presenter_) return {};

    const auto cpu = presenter_->cpu();
    return ": " + QString::number(cpu.usage_percent, 2, 2) + "%";
}

QString system_monitor_qt::cpu_frequency_mhz() const {
    if (!presenter_) return {};

    const auto cpu = presenter_->cpu();
    return QString::number(cpu.frequency_mhz, 2, 2) + "Mhz";
}

QString system_monitor_qt::cpu_temperature_c() const {
    if (!presenter_) return {};

    const auto cpu = presenter_->cpu();
    return QString::number(cpu.temperature_c / 1000) + "°C";
}

QString system_monitor_qt::cpu_power_mw() const {
    if (!presenter_) return {};

    const auto cpu = presenter_->cpu();
    return QString::number(cpu.temperature_c / 1000) + "mW";
}

QString system_monitor_qt::memory_vram_used() const {
    if (!presenter_) return {};

    const auto memory = presenter_->memory();
    return QString::number(memory.vram_used);
}

QString system_monitor_qt::memory_total_bytes() const {
    if (!presenter_) return {};

    const auto memory = presenter_->memory();
    return QString::number(memory.vram_total, 2, 1);
}

QString system_monitor_qt::memory_used_bytes() const {
    if (!presenter_) return {};

    const auto memory = presenter_->memory();
    return QString::number(memory.vram_used, 2, 1);
}

QString system_monitor_qt::memory_usage_percent() const {
    if (!presenter_) return {};

    const auto memory = presenter_->memory();
    return ": "+ QString::number(memory.usage_percent, 2, 2) + "%";
}

QString system_monitor_qt::memory_name() const {
    if (!presenter_) return {};

    const auto memory = presenter_->memory();
    return QString::fromStdString(memory.name);
}

QString system_monitor_qt::memory_power_mw() const {
    if (!presenter_) return {};

    const auto memory = presenter_->memory();
    return QString::fromStdString(memory.name) + "mW";
}

QString system_monitor_qt::memory_frequency_mhz() const {
    if (!presenter_) return {};

    const auto memory = presenter_->memory();
    return QString::fromStdString(memory.name) + "mhz";
}

/// gpu
QString system_monitor_qt::gpu_name() const {
    if (!presenter_) return {};

    const auto gpu = presenter_->gpu();
    return QString::fromStdString(gpu.name);
}

QString system_monitor_qt::gpu_vram_total() const {
    if (!presenter_) return {};

    const auto gpu = presenter_->gpu();
    return QString::number(gpu.vram_total / 1024);
}

QString system_monitor_qt::gpu_vram_used() const {
    if (!presenter_) return {};

    const auto gpu = presenter_->gpu();
    return QString::number(gpu.vram_used / 1024);
}

QString system_monitor_qt::gpu_usage_percent() const {
    if (!presenter_) return {};

    const auto gpu = presenter_->gpu();
    return ": " + QString::number(gpu.usage_percent) + "%";
}

QString system_monitor_qt::gpu_cores() const {
    if (!presenter_) return {};

    const auto gpu = presenter_->gpu();
    return QString::number(gpu.cores);
}

QString system_monitor_qt::gpu_frequency_mhz() const {
    if (!presenter_) return {};

    const auto gpu = presenter_->gpu();
    return QString::number(gpu.frequency_mhz) + "mhz";
}

QString system_monitor_qt::gpu_temperature_c() const {
    if (!presenter_) return {};

    const auto gpu = presenter_->gpu();
    return QString::number(gpu.temperature_c) + "°C";
}

/// disk
QString system_monitor_qt::disk_read_speed() const {
    if (!presenter_) return {};

    const auto disk = presenter_->disk();
    return QString::number(disk.read_speed);
}

QString system_monitor_qt::disk_write_speed() const {
    if (!presenter_) return {};

    const auto disk = presenter_->disk();
    return QString::number(disk.write_speed);
}

QString system_monitor_qt::disk_sector_size() const {
    if (!presenter_) return {};

    const auto disk = presenter_->disk();
    return QString::number(disk.sector_size);
}

QString system_monitor_qt::disk_model() const {
    if (!presenter_) return {};

    const auto disk = presenter_->disk();
    return QString::fromStdString(disk.model);
}

QString system_monitor_qt::disk_serial_number() const {
    if (!presenter_) return {};

    const auto disk = presenter_->disk();
    return QString::fromStdString(disk.serial_number);
}

QString system_monitor_qt::disk_size() const {
    if (!presenter_) return {};

    const auto disk = presenter_->disk();
    return QString::number(disk.size);
}

QString system_monitor_qt::disk_usage_percent() const {
    if (!presenter_) return {};

    const auto disk = presenter_->disk();
    return ": " + QString::number((100 * disk.size)) + "%";
}

/// net
QString system_monitor_qt::net_rx_bytes() const {
    if (!presenter_) return {};

    const auto net = presenter_->net();
    return QString::number(net.rx_bytes);
}

QString system_monitor_qt::net_tx_bytes() const {
    if (!presenter_) return {};
    const auto net = presenter_->net();
    return QString::number(net.tx_bytes);
}


void system_monitor_qt::refresh_ui() {
    if (!presenter_) return;

    auto current_cpu = presenter_->cpu();
    if (current_cpu != last_cpu_) {
        last_cpu_ = current_cpu;
        emit cpu_changed();
    }

    auto current_memory = presenter_->memory();
    if (current_memory != last_memory_) {
        last_memory_ = current_memory;
        emit memory_changed();
    }

    auto current_gpu = presenter_->gpu();
    if (current_gpu != last_gpu_) {
        last_gpu_ = current_gpu;
        emit gpu_changed();
    }

    auto current_disk = presenter_->disk();
    if (current_disk != last_disk_) {
        last_disk_ = current_disk;
        emit disk_changed();
    }

    auto current_net = presenter_->net();
    if (current_net != last_net_) {
        last_net_ = current_net;
        emit net_changed();
    }
}

} // namespace qt
} // namespace ui
