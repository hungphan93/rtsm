#include "system_monitor_qt.hpp"

namespace ui {
namespace qt {

system_monitor_qt::system_monitor_qt(presenter::system_monitor* presenter,QObject* parent) :
    QObject(parent), presenter_(presenter) {
}

QString system_monitor_qt::cpu_model_name() const {
    if (!presenter_) return {};

    const auto cpu = presenter_->cpu();
    return QString::fromStdString(cpu.model_name);
}

QString system_monitor_qt::cpu_usage_percent() const {
    if (!presenter_) return {};

    const auto cpu = presenter_->cpu();
    return QString::number(cpu.usage_percent);
}

QString system_monitor_qt::cpu_frequency_mhz() const {
    if (!presenter_) return {};

    const auto cpu = presenter_->cpu();
    return QString::number(cpu.frequency_mhz);
}

QString system_monitor_qt::cpu_temperature_c() const {
    if (!presenter_) return {};

    const auto cpu = presenter_->cpu();
    return QString::number(cpu.temperature_c);
}

} // namespace qt
} // namespace ui
