/// MIT License
#ifndef UI_QT_SYSTEM_MONITOR_VIEW_QT_HPP
#define UI_QT_SYSTEM_MONITOR_VIEW_QT_HPP

#include <QObject>
#include <QTimer>
#include <memory>
#include "system_monitor_backend_fwd.hpp"

// Khai báo trước (forward declaration) để Qt biết có 1 cái hộp sỏ tồn tại
// nhưng Qt không cần (và không được) biết bên trong nó có gì.
// struct system_monitor_backend_engine;
// // Hàm Factory "nhà máy" chế tạo hộp sỏ. (Vẫn né hệ thống C++20 Module)
// std::unique_ptr<system_monitor_backend_engine> create_system_monitor_backend();

namespace ui::qt {

/// This class is controller in MVC pattern
class system_monitor_view_qt : public QObject {

    Q_OBJECT
    /// cpu
    Q_PROPERTY(QString cpu_model_name READ cpu_model_name NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_usage_percent READ cpu_usage_percent NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_frequency_mhz READ cpu_frequency_mhz NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_temperature_c READ cpu_temperature_c NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_power READ cpu_power NOTIFY cpu_changed)

    /// memory
    Q_PROPERTY(QString memory_vram_used READ memory_vram_used NOTIFY memory_changed)
    Q_PROPERTY(QString memory_total_bytes READ memory_total_bytes NOTIFY memory_changed)
    Q_PROPERTY(QString memory_used_bytes READ memory_used_bytes NOTIFY memory_changed)
    Q_PROPERTY(QString memory_usage_percent READ memory_usage_percent NOTIFY memory_changed)
    Q_PROPERTY(QString memory_name READ memory_name NOTIFY memory_changed)
    Q_PROPERTY(QString memory_voltage READ memory_voltage NOTIFY memory_changed)
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
    //explicit system_monitor_view_qt(presenter::system_monitor_presenter& view_model, QObject* parent = nullptr);
    explicit system_monitor_view_qt(std::shared_ptr<system_monitor_backend_engine> backend, QObject* parent = nullptr);
    ~system_monitor_view_qt();
    /// cpu
    [[nodiscard]] QString cpu_model_name() const;
    [[nodiscard]] QString cpu_usage_percent() const;
    [[nodiscard]] QString cpu_frequency_mhz() const;
    [[nodiscard]] QString cpu_temperature_c() const;
    [[nodiscard]] QString cpu_power() const;

    /// memory
    [[nodiscard]] QString memory_vram_used() const;
    [[nodiscard]] QString memory_total_bytes() const;
    [[nodiscard]] QString memory_used_bytes() const;
    [[nodiscard]] QString memory_usage_percent() const;
    [[nodiscard]] QString memory_name() const;
    [[nodiscard]] QString memory_voltage() const;
    [[nodiscard]] QString memory_frequency_mhz() const;

    /// gpu
    [[nodiscard]] QString gpu_name() const;
    [[nodiscard]] QString gpu_vram_total() const;
    [[nodiscard]] QString gpu_vram_used() const;
    [[nodiscard]] QString gpu_usage_percent() const;
    [[nodiscard]] QString gpu_cores() const;
    [[nodiscard]] QString gpu_frequency_mhz() const;
    [[nodiscard]] QString gpu_temperature_c() const;

    /// disk
    [[nodiscard]] QString disk_read_speed() const;
    [[nodiscard]] QString disk_write_speed() const;
    [[nodiscard]] QString disk_sector_size() const;
    [[nodiscard]] QString disk_model() const;
    [[nodiscard]] QString disk_serial_number() const;
    [[nodiscard]] QString disk_size() const;
    [[nodiscard]] QString disk_usage_percent() const;


    /// net
    [[nodiscard]] QString net_rx_bytes() const;
    [[nodiscard]] QString net_tx_bytes() const;


signals:
    void cpu_changed();
    void memory_changed();
    void disk_changed();
    void gpu_changed();
    void net_changed();

private:
    //presenter::system_monitor_presenter& view_model_;
    //  std::unique_ptr<system_monitor_presenter_pimpl> view_model_;
    // Pimpl bây giờ chính là cục backend hộp đen đó!
    std::shared_ptr<system_monitor_backend_engine> backend_;
};

} /// namespace

#endif /// UI_QT_SYSTEM_MONITOR_View_QT_HPP