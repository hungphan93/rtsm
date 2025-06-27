#ifndef UI_QT_SYSTEM_MONITOR_QT_HPP
#define UI_QT_SYSTEM_MONITOR_QT_HPP
#include "presenter/system_monitor.hpp"

#include <QObject>

namespace ui {
namespace qt {

class system_monitor_qt : public QObject {

    Q_OBJECT
    Q_PROPERTY(QString cpu_model_name READ cpu_model_name NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_usage_percent READ cpu_usage_percent NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_frequency_mhz READ cpu_frequency_mhz NOTIFY cpu_changed)
    Q_PROPERTY(QString cpu_temperature_c READ cpu_temperature_c NOTIFY cpu_changed)

public:
    explicit system_monitor_qt(presenter::system_monitor* presenter = nullptr,QObject* parent = nullptr);

    QString cpu_model_name() const;
    QString cpu_usage_percent() const;
    QString cpu_frequency_mhz() const;
    QString cpu_temperature_c() const;

signals:
    void cpu_changed();

private:
    presenter::system_monitor* presenter_ = nullptr;

    QString model_name_;
    QString usage_percent_;
    QString frequency_mhz_;
    QString temperature_c_;
};

} // namespace qt
} // namespace ui

#endif // UI_QT_SYSTEM_MONITOR_QT_HPP
