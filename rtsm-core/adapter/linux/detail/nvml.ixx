/// MIT License
module;

#include <dlfcn.h>

export module adapter:nvml;

import std;

/// Exported API
export namespace adapter::linux2::nvml
{

struct gpu_info {
	unsigned long long vram_total_mb = 0;
	unsigned long long vram_used_mb = 0;
	unsigned int temperature_c = 0;
	unsigned int clock_mhz = 0;
	std::string name;
};

std::optional<gpu_info> read();

} /// namespace adapter::linux2::nvml

/// Internal
namespace adapter::linux2::nvml
{

typedef enum {
	OK = 0
} ret;

typedef struct nvmlDevice_st *dev;

typedef struct {
	unsigned long long total, free, used;
} mem;

using fn_init = ret (*)(void);
using fn_shut = ret (*)(void);
using fn_dev = ret (*)(unsigned, dev *);
using fn_mem = ret (*)(dev, mem *);
using fn_temp = ret (*)(dev, int, unsigned *);
using fn_clock = ret (*)(dev, int, unsigned *);
using fn_name = ret (*)(dev, char *, unsigned);

static void *dl = nullptr;
static bool tried = false;

static fn_shut shut = nullptr;
static fn_dev gdev = nullptr;
static fn_mem gmem = nullptr;
static fn_temp gtemp = nullptr;
static fn_clock gclk = nullptr;
static fn_name gname = nullptr;

struct guard {
	~guard()
	{
		if (dl) {
			if (shut)
				shut();
			dlclose(dl);
		}
	}
};
static guard g;

bool load()
{
	if (tried)
		return dl != nullptr;
	tried = true;

	dl = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
	if (!dl)
		dl = dlopen("libnvidia-ml.so", RTLD_LAZY);
	if (!dl)
		return false;

	auto init = (fn_init)dlsym(dl, "nvmlInit_v2");
	shut = (fn_shut)dlsym(dl, "nvmlShutdown");
	gdev = (fn_dev)dlsym(dl, "nvmlDeviceGetHandleByIndex_v2");
	gmem = (fn_mem)dlsym(dl, "nvmlDeviceGetMemoryInfo");
	gtemp = (fn_temp)dlsym(dl, "nvmlDeviceGetTemperature");
	gclk = (fn_clock)dlsym(dl, "nvmlDeviceGetClockInfo");
	gname = (fn_name)dlsym(dl, "nvmlDeviceGetName");

	if (!init || !shut || !gdev || !gmem || !gtemp || !gclk || init() != OK) {
		dlclose(dl);
		dl = nullptr;
		return false;
	}

	return true;
}

std::optional<gpu_info> read()
{
	if (!load())
		return std::nullopt;

	dev device;
	if (gdev(0, &device) != OK)
		return std::nullopt;

	gpu_info info;

	if (gname) {
		char buf[96] = {};
		if (gname(device, buf, sizeof(buf)) == OK)
			info.name = buf;
	}

	mem m;
	if (gmem(device, &m) == OK) {
		info.vram_total_mb = m.total / (1024 * 1024);
		info.vram_used_mb = m.used / (1024 * 1024);
	}

	unsigned v = 0;
	if (gtemp(device, 0, &v) == OK)
		info.temperature_c = v;

	v = 0;
	if (gclk(device, 0, &v) == OK)
		info.clock_mhz = v;

	return info;
}

} // namespace adapter::linux2::nvml
