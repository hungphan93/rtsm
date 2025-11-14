/// MIT License
#ifndef UI_QT_SYSTEM_MONITOR_QT_HPP
#define UI_QT_SYSTEM_MONITOR_QT_HPP
#include "presenter/system_monitor.hpp"

#include <QObject>
#include <QTimer>

namespace ui {
namespace qt {

class system_monitor_qt : public QObject {

    Q_OBJECT
    /// cpu
    Q_PROPERTY(QString cpu_model_name READ cpu_model_name NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_usage_percent READ cpu_usage_percent NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_frequency_mhz READ cpu_frequency_mhz NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_temperature_c READ cpu_temperature_c NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_power_mw READ cpu_power_mw NOTIFY cpu_changed)

    /// memory
    Q_PROPERTY(QString memory_vram_used READ memory_vram_used NOTIFY memory_changed)
    Q_PROPERTY(QString memory_total_bytes READ memory_total_bytes NOTIFY memory_changed)
    Q_PROPERTY(QString memory_used_bytes READ memory_used_bytes NOTIFY memory_changed)
    Q_PROPERTY(QString memory_usage_percent READ memory_usage_percent NOTIFY memory_changed)
    Q_PROPERTY(QString memory_name READ memory_name NOTIFY memory_changed)
    Q_PROPERTY(QString memory_power_mw READ memory_power_mw NOTIFY memory_changed)
    Q_PROPERTY(QString memory_frequency_mhz READ memory_frequency_mhz NOTIFY memory_changed)

    /// gpu
    Q_PROPERTY(QString gpu_name READ gpu_name NOTIFY gpu_changed)
    Q_PROPERTY(QString gpu_vram_total READ gpu_vram_total NOTIFY gpu_changed)
    Q_PROPERTY(QString gpu_vram_used READ gpu_vram_used NOTIFY gpu_changed)
    Q_PROPERTY(QString gpu_usage_percent READ gpu_usage_percent NOTIFY gpu_changed)
    Q_PROPERTY(QString gpu_cores READ gpu_cores NOTIFY gpu_changed)
    Q_PROPERTY(QString gpu_frequency_mhz READ gpu_frequency_mhz NOTIFY gpu_changed)
    Q_PROPERTY(QString gpu_temperature_c READ gpu_temperature_c NOTIFY gpu_changed)

    /// disk
    Q_PROPERTY(QString disk_usage_percent READ disk_usage_percent NOTIFY disk_changed)
    Q_PROPERTY(QString disk_read_speed READ disk_read_speed NOTIFY disk_changed)
    Q_PROPERTY(QString disk_write_speed READ disk_write_speed NOTIFY disk_changed)
    Q_PROPERTY(QString disk_sector_size READ disk_sector_size NOTIFY disk_changed)
    Q_PROPERTY(QString disk_model READ disk_model NOTIFY disk_changed)
    Q_PROPERTY(QString disk_serial_number READ disk_serial_number NOTIFY disk_changed)
    Q_PROPERTY(QString disk_size READ disk_size NOTIFY disk_changed)

    /// net
    Q_PROPERTY(QString net_rx_bytes READ net_rx_bytes NOTIFY net_changed)
    Q_PROPERTY(QString net_tx_bytes READ net_tx_bytes NOTIFY net_changed)

public:
    explicit system_monitor_qt(presenter::system_monitor* presenter = nullptr, QObject* parent = nullptr);

    /// cpu
    QString cpu_model_name() const;
    QString cpu_usage_percent() const;
    QString cpu_frequency_mhz() const;
    QString cpu_temperature_c() const;
    QString cpu_power_mw() const;

    /// memory
    QString memory_vram_used() const;
    QString memory_total_bytes() const;
    QString memory_used_bytes() const;
    QString memory_usage_percent() const;
    QString memory_name() const;
    QString memory_power_mw() const;
    QString memory_frequency_mhz() const;

    /// gpu
    QString gpu_name() const;
    QString gpu_vram_total() const;
    QString gpu_vram_used() const;
    QString gpu_usage_percent() const;
    QString gpu_cores() const;
    QString gpu_frequency_mhz() const;
    QString gpu_temperature_c() const;

    /// disk
    QString disk_read_speed() const;
    QString disk_write_speed() const;
    QString disk_sector_size() const;
    QString disk_model() const;
    QString disk_serial_number() const;
    QString disk_size() const;
    QString disk_usage_percent() const;


    /// net
    QString net_rx_bytes() const;
    QString net_tx_bytes() const;


signals:
    void cpu_changed();
    void memory_changed();
    void disk_changed();
    void gpu_changed();
    void net_changed();

private:
    void refresh_ui();
    presenter::system_monitor* presenter_ = nullptr;
    QTimer timer_;
    entity::cpu last_cpu_;
    entity::memory last_memory_;
    entity::disk last_disk_;
    entity::net last_net_;
    entity::gpu last_gpu_;
};

} // namespace qt
} // namespace ui

#endif // UI_QT_SYSTEM_MONITOR_QT_HPP
