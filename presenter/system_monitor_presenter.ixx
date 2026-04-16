/// MIT License
export module presenter:system_monitor_presenter;

import std;
import usecase;
import entity;
import :system_monitor_view_model;

export namespace presenter {

class system_monitor_presenter : public usecase::system_monitor_output_boundary {
public:
    using notify = std::function<void()>;

    system_monitor_presenter() noexcept = default;

    /// Provide the prepared ViewModel for the View layer
    [[nodiscard]] std::shared_ptr<const cpu_view_model> cpu_vm() const noexcept       { return get(cpu_); }
    [[nodiscard]] std::shared_ptr<const memory_view_model> memory_vm() const noexcept { return get(memory_); }
    [[nodiscard]] std::shared_ptr<const gpu_view_model> gpu_vm() const noexcept       { return get(gpu_); }
    [[nodiscard]] std::shared_ptr<const disk_view_model> disk_vm() const noexcept     { return get(disk_); }
    [[nodiscard]] std::shared_ptr<const net_view_model> net_vm() const noexcept       { return get(net_); }

    void on_cpu_changed(notify cb)    { set_callback(cpu_, std::move(cb)); }
    void on_memory_changed(notify cb) { set_callback(memory_, std::move(cb)); }
    void on_gpu_changed(notify cb)    { set_callback(gpu_, std::move(cb)); }
    void on_disk_changed(notify cb)   { set_callback(disk_, std::move(cb)); }
    void on_net_changed(notify cb)    { set_callback(net_, std::move(cb)); }

private:
    // ---------------------------------------------------------
    // FORMATTING UTILITY FUNCTIONS (STANDARDIZED TO RAW BYTES)
    // ---------------------------------------------------------

    /// Specifically designed for SPEED (Network, Disk I/O)
    static std::string format_speed(double bytes_per_sec) {
        int unit = 0;
        const char* units[] = {"B/s", "KB/s", "MB/s", "GB/s", "TB/s"};
        while (bytes_per_sec >= 1024.0 && unit < 4) {
            bytes_per_sec /= 1024.0;
            unit++;
        }
        char buffer[32];
        int precision = unit >= 3 ? 2 : 1;
        std::snprintf(buffer, sizeof(buffer), "%.*f%s", precision, bytes_per_sec, units[unit]);
        return std::string(buffer);
    }

    /// Specifically designed for CAPACITY (RAM, VRAM, Disk Size)
    static std::string format_bytes(double bytes) {
        int unit = 0;
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        while (bytes >= 1024.0 && unit < 4) {
            bytes /= 1024.0;
            unit++;
        }
        char buffer[32];
        int precision = unit >= 3 ? 2 : 1;
        std::snprintf(buffer, sizeof(buffer), "%.*f%s", precision, bytes, units[unit]);
        return std::string(buffer);
    }

    static std::string format_percent(float value) {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%.2f%%", value);
        return std::string(buffer);
    }

    static std::string format_float(float value, int precision, const std::string& suffix = "") {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%.*f%s", precision, value, suffix.c_str());
        return std::string(buffer);
    }

    // ---------------------------------------------------------
    // IMPLEMENT OUTPUT BOUNDARY
    // ---------------------------------------------------------
    void present_cpu(const entity::cpu& v) override {
        cpu_view_model vm;
        vm.model_name    = v.model_name;
        vm.usage_percent = format_percent(v.usage_percent);
        vm.frequency_mhz = format_float(v.frequency_mhz, 0, "MHz");
        vm.temperature_c = format_float(v.temperature_c, 0, "°C");
        vm.power         = format_float(v.power_uw / 1000000.0f, 1, "W");
        update(cpu_, vm);
    }

    void present_memory(const entity::memory& v) override {
        memory_view_model vm;
        vm.name          = v.name;

        // FIX: The entity stores RAM in GiB. We must multiply by 1024^3 to restore it to Raw Bytes
        vm.vram_total    = format_bytes(v.vram_total * 1024.0 * 1024.0 * 1024.0);
        vm.vram_used     = format_bytes(v.vram_used * 1024.0 * 1024.0 * 1024.0) + " of ";

        vm.usage_percent = format_percent(v.usage_percent);
        vm.voltage       = format_float(v.voltage, 1, "V");
        vm.frequency_mhz = format_float(v.frequency_mhz, 0, "MHz");
        update(memory_, vm);
    }

    void present_gpu(const entity::gpu& v) override {
        gpu_view_model vm;
        vm.name          = v.name;

        // FIX: GPU VRAM is stored in MiB. Multiply by 1024^2 to restore it to Raw Bytes
        vm.vram_total    = format_bytes(v.vram_total * 1024.0 * 1024.0);
        vm.vram_used     = format_bytes(v.vram_used * 1024.0 * 1024.0) + " of ";

        vm.usage_percent = format_percent(v.usage_percent);
        vm.cores         = std::to_string(v.cores);
        vm.frequency_mhz = format_float(v.frequency_mhz, 0, "MHz");
        vm.temperature_c = format_float(v.temperature_c, 0, "°C");
        update(gpu_, vm);
    }

    void present_disk(const entity::disk& v) override {
        disk_view_model vm;
        vm.model         = v.model;
        vm.serial_number = v.serial_number;

        // FIX: v.size is currently 0, the actual capacity relies on v.total (GiB). Multiply by 1024^3
        vm.size          = format_bytes(v.total * 1024.0 * 1024.0 * 1024.0);

        vm.read_speed    = format_speed(v.read_speed);  // Already calculated as Bytes/s in the Adapter
        vm.write_speed   = format_speed(v.write_speed); // Already calculated as Bytes/s in the Adapter
        vm.usage_percent = v.total > 0 ? format_percent(100.0f * v.used / v.total) : "0.00%";
        vm.sector_size   = std::to_string(v.sector_size);
        update(disk_, vm);
    }

    void present_net(const entity::net& v) override {
        net_view_model vm;
        vm.rx_speed = format_speed(v.rx_bytes); // Already calculated as Bytes/s in the Adapter
        vm.tx_speed = format_speed(v.tx_bytes); // Already calculated as Bytes/s in the Adapter
        update(net_, vm);
    }

    // ---------------------------------------------------------
    // CORE THREAD-SAFE ENGINE
    // ---------------------------------------------------------
    template<typename T>
    struct field {
        std::shared_ptr<const T> data = std::make_shared<T>();
        notify cb;
        mutable std::mutex mtx;
    };

    template<typename T>
    [[nodiscard]] std::shared_ptr<const T> get(const field<T>& f) const noexcept {
        std::scoped_lock lock(f.mtx);
        return f.data;
    }

    template<typename T>
    void set_callback(field<T>& f, notify cb) {
        std::scoped_lock lock(f.mtx);
        f.cb = std::move(cb);
    }

    template<typename T>
    void update(field<T>& f, const T& v) {
        auto new_data = std::make_shared<T>(v);
        notify local_cb;
        {
            std::scoped_lock lock(f.mtx);
            // Prevent unnecessary UI redraws if the data has not changed
            if (f.data && *f.data == v) return;
            f.data = std::move(new_data);
            local_cb = f.cb;
        }

        /// Invoke the callback outside the lock to avoid potential deadlocks
        if (local_cb) local_cb();
    }

    field<cpu_view_model> cpu_;
    field<memory_view_model> memory_;
    field<gpu_view_model> gpu_;
    field<disk_view_model> disk_;
    field<net_view_model> net_;
};

} /// namespace presenter
