module;

#include <dlfcn.h> /// For libnvidia-ml.so dynamic loading
#include <mntent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/sysmacros.h>

module adapter;
import std;

namespace adapter::linux2
{

///=============================================================================
/// NVML Dynamic Loading Wrapper
///=============================================================================
namespace nvml
{
typedef enum nvmlReturn_enum {
	NVML_SUCCESS = 0
} nvmlReturn_t;
typedef struct nvmlDevice_st *nvmlDevice_t;
typedef struct nvmlMemory_st {
	unsigned long long total;
	unsigned long long free;
	unsigned long long used;
} nvmlMemory_t;

typedef nvmlReturn_t (*nvmlInit_t)(void);
typedef nvmlReturn_t (*nvmlShutdown_t)(void);
typedef nvmlReturn_t (*nvmlDeviceGetHandleByIndex_v2_t)(unsigned int, nvmlDevice_t *);
typedef nvmlReturn_t (*nvmlDeviceGetMemoryInfo_t)(nvmlDevice_t, nvmlMemory_t *);
typedef nvmlReturn_t (*nvmlDeviceGetTemperature_t)(nvmlDevice_t, int, unsigned int *);
typedef nvmlReturn_t (*nvmlDeviceGetClockInfo_t)(nvmlDevice_t, int, unsigned int *);
typedef nvmlReturn_t (*nvmlDeviceGetUtilizationRates_t)(
	nvmlDevice_t,
	void *); // Placeholder, actual struct needed
typedef nvmlReturn_t (*nvmlDeviceGetName_t)(nvmlDevice_t, char *, unsigned int);
typedef nvmlReturn_t (*nvmlDeviceGetCount_v2_t)(unsigned int *);

static void *handle = nullptr;
static bool loaded = false;
static bool init_attempted = false;

static nvmlInit_t nvmlInit_v2_ptr = nullptr;
static nvmlDeviceGetHandleByIndex_v2_t nvmlDeviceGetHandle_ptr = nullptr;
static nvmlDeviceGetUtilizationRates_t nvmlDeviceGetUtilizationRates_ptr = nullptr;
static nvmlDeviceGetMemoryInfo_t nvmlDeviceGetMemoryInfo_ptr = nullptr;
static nvmlDeviceGetClockInfo_t nvmlDeviceGetClockInfo_ptr = nullptr;
static nvmlDeviceGetTemperature_t nvmlDeviceGetTemperature_ptr = nullptr;
static nvmlDeviceGetName_t nvmlDeviceGetName_ptr = nullptr;
static nvmlShutdown_t nvmlShutdown_ptr = nullptr;
static nvmlDeviceGetCount_v2_t nvmlDeviceGetCount_v2_ptr = nullptr;

struct nvml_guard {
	~nvml_guard()
	{
		if (handle) {
			if (nvmlShutdown_ptr) {
				nvmlShutdown_ptr();
			}
			dlclose(handle);
			handle = nullptr;
		}
	}
};

static nvml_guard guard;

bool load_nvml()
{
	if (init_attempted)
		return loaded;
	init_attempted = true;

	handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
	if (!handle)
		handle = dlopen("libnvidia-ml.so", RTLD_LAZY);
	if (!handle)
		return false;

	nvmlInit_v2_ptr = (nvmlInit_t)dlsym(handle, "nvmlInit_v2");
	nvmlShutdown_ptr = (nvmlShutdown_t)dlsym(handle, "nvmlShutdown");
	nvmlDeviceGetHandle_ptr = (nvmlDeviceGetHandleByIndex_v2_t)
		dlsym(handle, "nvmlDeviceGetHandleByIndex_v2");
	nvmlDeviceGetMemoryInfo_ptr = (nvmlDeviceGetMemoryInfo_t)dlsym(handle,
								       "nvmlDeviceGetMemoryInfo");
	nvmlDeviceGetTemperature_ptr = (nvmlDeviceGetTemperature_t)
		dlsym(handle, "nvmlDeviceGetTemperature");
	nvmlDeviceGetClockInfo_ptr = (nvmlDeviceGetClockInfo_t)dlsym(handle,
								     "nvmlDeviceGetClockInfo");

	// Optional pointers (we don't fail if these are missing, as they are not currently used in read_gpu)
	nvmlDeviceGetUtilizationRates_ptr = (nvmlDeviceGetUtilizationRates_t)
		dlsym(handle, "nvmlDeviceGetUtilizationRates");
	nvmlDeviceGetName_ptr = (nvmlDeviceGetName_t)dlsym(handle, "nvmlDeviceGetName");
	nvmlDeviceGetCount_v2_ptr = (nvmlDeviceGetCount_v2_t)dlsym(handle, "nvmlDeviceGetCount_v2");

	if (!nvmlInit_v2_ptr || !nvmlShutdown_ptr || !nvmlDeviceGetHandle_ptr ||
	    !nvmlDeviceGetMemoryInfo_ptr || !nvmlDeviceGetTemperature_ptr ||
	    !nvmlDeviceGetClockInfo_ptr) {
		dlclose(handle);
		handle = nullptr;
		loaded = false;
	} else if (nvmlInit_v2_ptr() == NVML_SUCCESS) {
		loaded = true;
	} else {
		dlclose(handle);
		handle = nullptr;
		loaded = false;
	}
	return loaded;
}
}

system_info_reader_linux::system_info_reader_linux() noexcept
	: proc_cpu_{ "/proc/cpuinfo" }
	, proc_ram_{ "/proc/meminfo" }
{}

entity::cpu system_info_reader_linux::read_cpu() const
{
	entity::cpu result{};

	proc_cpu_.clear();
	proc_cpu_.seekg(0);

	std::string line;

	while (std::getline(proc_cpu_, line)) {
		auto value = detail::extract_value(line);

		if (line.contains("model name")) {
			std::cmatch match;
			if (std::regex_search(value.data(),
					      value.data() + value.size(),
					      match,
					      detail::cpu_name_pattern)) {
				result.model_name = match.str();
			} else {
				result.model_name = value;
			}
		} else if (line.contains("cache size"))
			detail::parse_first_number(result.l2_cache_kib, value);
		else if (line.contains("cpu cores"))
			detail::parse_first_number(result.cpu_cores, value);
		else if (line.contains("processor"))
			detail::parse_first_number(result.processor_id, value);
		else if (line.contains("physical id"))
			detail::parse_first_number(result.physical_id, value);
		else if (line.contains("siblings"))
			detail::parse_first_number(result.siblings, value);
		else if (line.contains("core id"))
			detail::parse_first_number(result.core_id, value);
	}

	/// temp cpu amd
	if (auto r = detail::find_hwmon_by_name("k10temp"); r) {
		hwmon_cpu_path_ = *r;
	}

	/// temp cpu intel
	if (auto r = detail::find_hwmon_by_name("coretemp"); r) {
		hwmon_cpu_path_ = *r;
	}

	/// temp gpu card onboard
	if (auto r = detail::find_hwmon_by_name("amdgpu"); r) {
		hwmon_gpu_path_ = *r;
	}

	result.usage_percent = 0.0f;

	auto times_result = detail::read_cpu_times();
	if (!times_result) {
		return result;
	}

	const auto &current_times = *times_result;

	const auto idle_diff = current_times.idle_time() - last_cpu_times_.idle_time();
	const auto total_diff = current_times.total() - last_cpu_times_.total();

	if (total_diff > 0) {
		result.usage_percent = detail::percent(total_diff - idle_diff, total_diff);
	}

	last_cpu_times_ = current_times;

	/// Read frequency mHz from the sysfs record using the read /proc/cpuinfo
	if (auto v = detail::read_cpu_frequency_avg("scaling_cur_freq"); v)
		result.frequency_mhz = *v;

	/// Using Cache Path dynamically to read CPU temp from k10temp (not loop all folder)
	if (auto v = detail::parse_number<float>(
		    detail::read_line(*hwmon_cpu_path_ / "temp1_input"));
	    v) {
		result.temperature_c = *v / 1000.f;
	}

	/// Using Cache Path dynamically to read power from amdgpu if integrated APU
	if (auto v = detail::parse_number<float>(
		    detail::read_line(*hwmon_gpu_path_ / "power1_input"));
	    v) {
		result.power_uw = *v;
	}

	return result;
}

/// https://docs.redhat.com/en/documentation/red_hat_enterprise_linux/6/html/deployment_guide/s2-proc-meminfo
entity::memory system_info_reader_linux::read_memory() const
{
	entity::memory result{};

	proc_ram_.clear();
	proc_ram_.seekg(0);

	std::string line;

	while (std::getline(proc_ram_, line)) {
		auto value = detail::extract_value(line);

		if (line.contains("MemTotal:"))
			detail::parse_first_number(result.vram_total, value);
		else if (line.contains("MemFree:"))
			detail::parse_first_number(result.vram_free, value);
		else if (line.contains("MemAvailable:"))
			detail::parse_first_number(result.vram_available, value);

		if (result.vram_total > 0 && result.vram_free > 0 && result.vram_available > 0) {
			break;
		}
	}

	/// KB -> MB -> GB
	result.vram_total = result.vram_total / (1024.0f * 1024.0f);
	result.vram_free = result.vram_free / (1024.0f * 1024.0f);

	result.vram_used = result.vram_total - result.vram_available / (1024.0f * 1024.0f);

	/// Using percent of RAM
	if (result.vram_used > 0 && result.vram_total > 0) {
		result.usage_percent = detail::percent(result.vram_used, result.vram_total);
	}

	/// https://linux.die.net/man/8/dmidecode
	std::string dmi_out = detail::exec_cmd("sudo -n dmidecode --type 17 2>/dev/null");

	std::istringstream stream(dmi_out);

	while (std::getline(stream, line)) {
		auto value = detail::extract_value(line);

		if (line.contains("Manufacturer:")) {
			result.name = value;
		}

		else if (line.contains("Configured Memory Speed:")) {
			float raw_speed = 0;
			detail::parse_first_number(raw_speed, value);
			result.frequency_mhz = raw_speed / 2;
		}

		else if (line.contains("Configured Voltage:") ||
			 line.contains("Maximum Voltage:")) {
			detail::parse_first_number(result.voltage, value);
		}
	}

	if (stream) {
		/// Print warning to configure sudo privileges
		std::clog << "Please run two command on terminal\n";
		std::clog
			<< R"(sudo sh -c 'echo "hungphan ALL=(ALL) NOPASSWD: /usr/sbin/dmidecode, /usr/bin/dmidecode" > /etc/sudoers.d/dmidecode')"
			<< "\n";
		std::clog << R"(sudo chmod 0440 /etc/sudoers.d/dmidecode)"
			  << "\n";
	}

	return result;
}

bool is_intel_integrated(std::uint16_t device_id)
{
	/// Arc (Discrete) → 0x56A0 - 0x56FF
	if (device_id >= 0x56A0 && device_id <= 0x56FF)
		return false;

	return true; /// Vendor Intel + không phải Arc = iGPU
}

bool is_amd_integrated(std::uint16_t device_id)
{
	/// APU / iGPU ranges
	if (device_id >= 0x1304 && device_id <= 0x131F)
		return true; /// Kabini APU
	if (device_id >= 0x1318 && device_id <= 0x131B)
		return true; /// Mullins APU
	if (device_id >= 0x15D8 && device_id <= 0x15DF)
		return true; /// Picasso / Raven2 (Ryzen 2000/3000G)
	if (device_id >= 0x15E7 && device_id <= 0x15FF)
		return true; /// Renoir (Ryzen 4000G)
	if (device_id >= 0x1636 && device_id <= 0x163F)
		return true; /// Renoir iGPU
	if (device_id >= 0x164C && device_id <= 0x164F)
		return true; /// Lucienne
	if (device_id >= 0x1681 && device_id <= 0x168F)
		return true; /// Rembrandt (Ryzen 6000) - RDNA2
	if (device_id >= 0x15BF && device_id <= 0x15C7)
		return true; /// Phoenix (Ryzen 7000) - RDNA3

	/// Discrete GPU ranges (Polaris, Vega, RDNA1/2/3)
	if (device_id >= 0x6600 && device_id <= 0x6FFF)
		return false; /// Polaris
	if (device_id >= 0x6800 && device_id <= 0x68FF)
		return false; /// Tahiti/GCN
	if (device_id >= 0x7300 && device_id <= 0x73FF)
		return false; /// RDNA2/3 dGPU

	return false;	      /// default is dGPU if not match
}

void system_info_reader_linux::classify_gpu(fs::path &hwmon_path, entity::gpu &result) const
{
	const auto vendor = static_cast<gpu_vendor>(result.vendor);

	switch (vendor) {
	case gpu_vendor::NVIDIA:
		read_nvidia_gpu(result);
		break;

	case gpu_vendor::AMD:
		is_amd_integrated((std::uint16_t)result.device) ? read_amd_igpu(hwmon_path, result)
							   : read_amd_dgpu(hwmon_path, result);
		break;

	case gpu_vendor::INTEL:
		break;

	/// Qualcomm Adreno on laptop ARM = iGPU
	case gpu_vendor::QUALCOMM:
		break;

	default:
		break;
	}
}

void system_info_reader_linux::read_nvidia_gpu(entity::gpu &result) const
{
	/// Fast path for NVIDIA using NVML C API
	if (!nvml::load_nvml())
		return;

	nvml::nvmlDevice_t device;
	if (nvml::nvmlDeviceGetHandle_ptr(0, &device) != nvml::NVML_SUCCESS)
		return;

	/// GPU name (written into cache so result.name stays up-to-date)
	if (nvml::nvmlDeviceGetName_ptr) {
		char name_buf[96] = {};
		if (nvml::nvmlDeviceGetName_ptr(device, name_buf, sizeof(name_buf)) ==
		    nvml::NVML_SUCCESS) {
			result.name = name_buf;
		}
	}

	/// VRAM
	nvml::nvmlMemory_t memory;
	if (nvml::nvmlDeviceGetMemoryInfo_ptr(device, &memory) == nvml::NVML_SUCCESS) {
		result.vram_total = memory.total / (1024 * 1024);
		result.vram_used = memory.used / (1024 * 1024);
		if (result.vram_total > 0) {
			result.usage_percent = detail::percent(result.vram_used, result.vram_total);
		}
	}

	/// Temperature  (0 = NVML_TEMPERATURE_GPU)
	unsigned int temp = 0;
	if (nvml::nvmlDeviceGetTemperature_ptr(device, 0, &temp) == nvml::NVML_SUCCESS) {
		result.temperature_c = temp;
	}

	/// Core clock  (0 = NVML_CLOCK_GRAPHICS)
	unsigned int freq = 0;
	if (nvml::nvmlDeviceGetClockInfo_ptr(device, 0, &freq) == nvml::NVML_SUCCESS) {
		result.frequency_mhz = freq;
	}
}

void system_info_reader_linux::read_amd_igpu(fs::path &hwmon_path, entity::gpu &result) const
{
	auto mem_total = detail::read_line(hwmon_path / "device/mem_info_vram_total");
	if (auto v = detail::to_uint(mem_total); v)
		result.vram_total = *v / (1024 * 1024);

	auto vram_used = detail::read_line(hwmon_path / "device/mem_info_vram_used");
	if (auto v = detail::to_uint(vram_used); v)
		result.vram_used = *v / (1024 * 1024);

	auto temperature_c = detail::read_line(hwmon_path / "temp1_input");
	if (auto v = detail::to_uint(temperature_c); v)
		result.temperature_c = *v / 1000.f;

	auto power = detail::read_line(hwmon_path / "power1_input");
	if (auto v = detail::to_uint(power); v)
		result.power = *v / 1000000.f;

	if (result.vram_used > 0 && result.vram_total > 0) {
		result.usage_percent = detail::percent(result.vram_used, result.vram_total);
	}

	auto frequency_mhz = detail::read_line(hwmon_path / "freq1_input");
	if (auto v = detail::to_uint(frequency_mhz); v)
		result.frequency_mhz = *v / 1000000.f;
}

void system_info_reader_linux::read_amd_dgpu(fs::path &hwmon_path, entity::gpu &result) const
{
	std::clog << "future support\n";
}

entity::gpu system_info_reader_linux::read_gpu() const
{
	entity::gpu result{};
	fs::path hwmon_path;

	if (auto r = detail::find_hwmon_by_name("amdgpu"); r) {
		hwmon_path = *r;
	}

	const auto vendor_str = detail::read_line(hwmon_path / "device/vendor");
	const auto device_str = detail::read_line(hwmon_path / "device/device");

	if (auto r = detail::to_uint<std::uint16_t>(vendor_str, 16); r) {
		result.vendor = *r;
	}

	if (auto r = detail::to_uint<std::uint16_t>(device_str, 16); r) {
		result.device = *r;
	}

	const auto vendor = static_cast<gpu_vendor>(result.vendor);

	if (vendor == gpu_vendor::NVIDIA) {
		result.name = "NVIDIA GPU";
	} else if (vendor == gpu_vendor::AMD) {
		result.name = "AMD Radeon Graphics";
	} else if (vendor == gpu_vendor::INTEL) {
		result.name = "Intel Graphics";
	} else {
		result.name = "Unknown GPU";
	}

	classify_gpu(hwmon_path, result);

	return result;
}

entity::disk system_info_reader_linux::read_disk() const
{
	entity::disk result{};
	std::uint64_t total_r = 0, total_w = 0;

	/// /sys/block: aggregate sectors + grab first disk model
	std::error_code ec;
	for (const auto &entry : fs::directory_iterator("/sys/block", ec)) {
		if (ec)
			break;
		const auto name = entry.path().filename().string();
		if (!name.starts_with("nvme") && !name.starts_with("sd"))
			continue;

		if (result.model.empty())
			result.model = detail::read_line(entry.path() / "device/model");

		std::istringstream iss(detail::read_line(entry.path() / "stat"));
		std::uint64_t a, b, r_sect, c, d, e, w_sect;
		iss >> a >> b >> r_sect >> c >> d >> e >> w_sect;
		total_r += r_sect;
		total_w += w_sect;
	}

	/// /proc/mounts + statvfs: aggregate total/used across real partitions
	std::unique_ptr<FILE, decltype(&endmntent)> mnt(setmntent("/proc/mounts", "r"), endmntent);
	std::unordered_set<std::string> seen;

	for (struct mntent *m; mnt && (m = getmntent(mnt.get()));) {
		std::string fs = m->mnt_fsname, ft = m->mnt_type;
		if (!fs.starts_with("/dev/") || fs.starts_with("/dev/loop"))
			continue;
		if (ft == "nfs" || ft == "nfs4" || ft == "cifs" || ft == "autofs" ||
		    ft.starts_with("fuse"))
			continue;
		if (!seen.insert(fs).second)
			continue;

		struct statvfs sv;
		if (statvfs(m->mnt_dir, &sv) != 0)
			continue;
		result.total += (double)sv.f_blocks * sv.f_frsize;
		result.used += (double)(sv.f_blocks - sv.f_bavail) * sv.f_frsize;
	}

	const auto current_time = std::chrono::steady_clock::now();
	const std::chrono::duration<double> dt = current_time - disk_prev_t_;
	const double dt_sec = dt.count();

	/// Convert cumulative sector deltas to bytes/sec by normalizing over elapsed time
	if (dt_sec > 0.001) {
		const std::uint64_t diff_r = (total_r >= disk_prev_r_) ? (total_r - disk_prev_r_) : 0;
		const std::uint64_t diff_w = (total_w >= disk_prev_w_) ? (total_w - disk_prev_w_) : 0;
		result.read_speed = (diff_r * 512.0) / dt_sec;
		result.write_speed = (diff_w * 512.0) / dt_sec;
	}

	/// update status
	disk_prev_t_ = current_time;
	disk_prev_r_ = total_r;
	disk_prev_w_ = total_w;

	return result;
}

entity::net system_info_reader_linux::read_net() const
{
	entity::net result{};

	auto get_total_net_bytes = []() -> std::pair<std::uint64_t, std::uint64_t> {
		std::ifstream proc("/proc/net/dev");
		if (!proc.is_open())
			return { 0, 0 };

		std::string line;
		std::getline(proc, line);
		std::getline(proc, line);

		std::uint64_t total_rx = 0;
		std::uint64_t total_tx = 0;

		while (std::getline(proc, line)) {
			auto colon_pos = line.find(':');
			if (colon_pos == std::string::npos)
				continue;

			std::string_view iface_view(line.data(), colon_pos);
			auto start = iface_view.find_first_not_of(" \t");
			if (start != std::string_view::npos) {
				iface_view = iface_view.substr(start);
			}

			if (iface_view == "lo")
				continue;

			std::istringstream iss(line.substr(colon_pos + 1));
			std::uint64_t rx = 0, tx = 0, skip;

			if (iss >> rx >> skip >> skip >> skip >> skip >> skip >> skip >> skip >>
			    tx) {
				total_rx += rx;
				total_tx += tx;
			}
		}
		return { total_rx, total_tx };
	};

	auto current_time = std::chrono::steady_clock::now();
	auto [current_rx, current_tx] = get_total_net_bytes();

	result.rx_bytes = net_prev_rx_;
	result.tx_bytes = net_prev_tx_;

	std::chrono::duration<double> dt = current_time - net_prev_t_;
	double dt_sec = dt.count();

	/// Convert cumulative sector deltas to bytes/sec by normalizing over elapsed time
	if (dt_sec > 0.001) {
		std::uint64_t diff_rx = (current_rx >= net_prev_rx_) ? (current_rx - net_prev_rx_) : 0;
		std::uint64_t diff_tx = (current_tx >= net_prev_tx_) ? (current_tx - net_prev_tx_) : 0;

		/// Raw Bytes/s
		result.rx_bytes = diff_rx / dt_sec;
		result.tx_bytes = diff_tx / dt_sec;
	}

	/// Update status
	net_prev_t_ = current_time;
	net_prev_rx_ = current_rx;
	net_prev_tx_ = current_tx;

	return result;
}

} /// namespace
