/// MIT License
#include "system_monitor_view_qt.hpp"
#include <memory>

import std;
import entity;
import usecase;
import presenter;

struct system_monitor_backend_engine {
    std::shared_ptr<presenter::system_monitor_presenter> smp;
};

std::shared_ptr<system_monitor_backend_engine> create_system_monitor_backend(std::any presenter_instance) {
    auto engine = std::make_shared<system_monitor_backend_engine>();
    engine->smp = std::any_cast<std::shared_ptr<presenter::system_monitor_presenter>>(presenter_instance);
    return engine;
}

// Define destructor ở đây, KHÔNG để = default trong header
//system_monitor_backend_engine::~system_monitor_backend_engine() = default;

namespace ui::qt {

//system_monitor_view_qt::system_monitor_view_qt(presenter::system_monitor_presenter& view_mode,QObject* parent) :
// system_monitor_view_qt::system_monitor_view_qt(QObject* parent) :
//     QObject(parent), backend_{std::make_unique<system_monitor_presenter_pimpl>()} {

//     auto bind = [this](auto register_fn, auto signal) {
//         (backend_->smp->*register_fn)([this, signal] {
//             QMetaObject::invokeMethod(this, signal, Qt::QueuedConnection);
//         });
//     };

//     bind(&presenter::system_monitor_presenter::on_cpu_changed,    &system_monitor_view_qt::cpu_changed);
//     bind(&presenter::system_monitor_presenter::on_memory_changed, &system_monitor_view_qt::memory_changed);
//     bind(&presenter::system_monitor_presenter::on_gpu_changed,    &system_monitor_view_qt::gpu_changed);
//     bind(&presenter::system_monitor_presenter::on_disk_changed,   &system_monitor_view_qt::disk_changed);
//     bind(&presenter::system_monitor_presenter::on_net_changed,    &system_monitor_view_qt::net_changed);
// }
// 3. Sự kết hợp hoàn mỹ ở Constructor (Bơm sẵn backend đã lắp ráp)
system_monitor_view_qt::system_monitor_view_qt(std::shared_ptr<system_monitor_backend_engine> backend, QObject* parent)
    : QObject(parent), backend_(std::move(backend)) {

    if (!backend_ || !backend_->smp) return;
    // Công thức Lambda đắt giá bẻ luồng Event:
    auto bind_signal = [this](auto register_fn, auto signal) {
        // Áp dụng con trỏ register_fn lên lõi object thật.
        // LƯU Ý LỖI HỒI NÃY CỦA BẠN: Dùng .get() để lấy con trỏ thô của shared_ptr ->*
        (backend_->smp.get()->*register_fn)([this, signal] {
            QMetaObject::invokeMethod(this, signal, Qt::QueuedConnection);
        });
    };
    // Móc Callback vào Qt Signals
    bind_signal(&presenter::system_monitor_presenter::on_cpu_changed,    &system_monitor_view_qt::cpu_changed);
    bind_signal(&presenter::system_monitor_presenter::on_memory_changed, &system_monitor_view_qt::memory_changed);
    bind_signal(&presenter::system_monitor_presenter::on_gpu_changed,    &system_monitor_view_qt::gpu_changed);
    bind_signal(&presenter::system_monitor_presenter::on_disk_changed,   &system_monitor_view_qt::disk_changed);
    bind_signal(&presenter::system_monitor_presenter::on_net_changed,    &system_monitor_view_qt::net_changed);
}

system_monitor_view_qt::~system_monitor_view_qt() = default;

QString system_monitor_view_qt::cpu_model_name() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->cpu_vm()->model_name);
}

QString system_monitor_view_qt::cpu_usage_percent() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->cpu_vm()->usage_percent);
}

QString system_monitor_view_qt::cpu_frequency_mhz() const {
    if (!backend_ || !backend_->smp) return {};
    return  QString::fromStdString(backend_->smp->cpu_vm()->frequency_mhz);
}

QString system_monitor_view_qt::cpu_temperature_c() const {
    if (!backend_ || !backend_->smp) return {};
    return  QString::fromStdString(backend_->smp->cpu_vm()->temperature_c);
}

QString system_monitor_view_qt::cpu_power() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->cpu_vm()->power);
}

QString system_monitor_view_qt::memory_vram_used() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->memory_vm()->vram_used);
}

QString system_monitor_view_qt::memory_total_bytes() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->memory_vm()->vram_total);
}

QString system_monitor_view_qt::memory_used_bytes() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->memory_vm()->vram_used);
}

QString system_monitor_view_qt::memory_usage_percent() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->memory_vm()->usage_percent);
}

QString system_monitor_view_qt::memory_name() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->memory_vm()->name);
}

QString system_monitor_view_qt::memory_voltage() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->memory_vm()->voltage);
}

QString system_monitor_view_qt::memory_frequency_mhz() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->memory_vm()->frequency_mhz);
}

/// gpu
QString system_monitor_view_qt::gpu_name() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->gpu_vm()->name);
}

QString system_monitor_view_qt::gpu_vram_total() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->gpu_vm()->vram_total);
}

QString system_monitor_view_qt::gpu_vram_used() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->gpu_vm()->vram_used);
}

QString system_monitor_view_qt::gpu_usage_percent() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->gpu_vm()->usage_percent);
}

QString system_monitor_view_qt::gpu_cores() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->gpu_vm()->cores);
}

QString system_monitor_view_qt::gpu_frequency_mhz() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->gpu_vm()->frequency_mhz);
}

QString system_monitor_view_qt::gpu_temperature_c() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->gpu_vm()->temperature_c);
}

/// disk
QString system_monitor_view_qt::disk_read_speed() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->disk_vm()->read_speed);

}

QString system_monitor_view_qt::disk_write_speed() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->disk_vm()->write_speed);
}

QString system_monitor_view_qt::disk_sector_size() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->disk_vm()->sector_size);
}

QString system_monitor_view_qt::disk_model() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->disk_vm()->model);
}

QString system_monitor_view_qt::disk_serial_number() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->disk_vm()->serial_number);
}

QString system_monitor_view_qt::disk_size() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->disk_vm()->size);
}

QString system_monitor_view_qt::disk_usage_percent() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->disk_vm()->usage_percent);
}

/// net
QString system_monitor_view_qt::net_rx_bytes() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->net_vm()->rx_speed);
}

QString system_monitor_view_qt::net_tx_bytes() const {
    if (!backend_ || !backend_->smp) return {};
    return QString::fromStdString(backend_->smp->net_vm()->tx_speed);
}

} /// namespace