#pragma once
#include <memory>
#include <any>

struct system_monitor_backend_engine;

std::shared_ptr<system_monitor_backend_engine> create_system_monitor_backend(std::any presenter_instance);
