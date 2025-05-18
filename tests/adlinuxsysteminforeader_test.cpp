#include <gtest/gtest.h>
#include "../adapters/adlinuxsysteminforeader.h"
#include "../entities/ecpuinfo.h"
// #include "../entities/ememoryinfo.h"
// #include "../entities/egpuinfo.h"
// #include "../entities/ediskinfo.h"
// #include "../entities/enetworkinfo.h"
// #include <thread>
// #include <chrono>

// TEST(ADLinuxSystemInfoReaderTest, ReadCpuTimes_NotZero) {
//     ADLinuxSystemInfoReader reader;
//     CpuTimes times = reader.readCpuTimes();
//     EXPECT_GT(times.total(), 0);
// }
TEST(ADLinuxSystemInfoReaderTest, ReadCpuModel_NotEmpty) {
    ADLinuxSystemInfoReader reader;
    ESystemInfo info = reader.read();  // assuming this calls readCpuTimes()
    EXPECT_FALSE(info.cpu.modelName.empty());
}


// TEST(ADLinuxSystemInfoReaderTest, ReadCpuUsagePercent_ReasonableValue) {
//     ADLinuxSystemInfoReader reader;
//     std::string usageStr = reader.readCpuUsagePercent();
//     ASSERT_FALSE(usageStr.empty());

//     double usage = std::stod(usageStr);
//     EXPECT_GE(usage, 0.0);
//     EXPECT_LE(usage, 100.0);
// }

// TEST(ADLinuxSystemInfoReaderTest, ReadCpuInfoFromProc_ModelNotEmpty) {
//     ADLinuxSystemInfoReader reader;
//     ESystemInfo sysInfo = reader.read();

//     EXPECT_FALSE(sysInfo.cpu.modelName.empty());
//     EXPECT_GT(sysInfo.cpu.coreNumber, 0);
//     EXPECT_GT(sysInfo.cpu.threads.size(), 0);
// }

// TEST(ADLinuxSystemInfoReaderTest, ReadMemoryInfo_ValidMemoryUsage) {
//     ADLinuxSystemInfoReader reader;
//     ESystemInfo sysInfo = reader.read();

//     EXPECT_GT(sysInfo.mem.total_bytes, 0);
//     EXPECT_GT(sysInfo.mem.used_bytes, 0);
//     EXPECT_FALSE(sysInfo.mem.usage_percent.empty());
// }

// TEST(ADLinuxSystemInfoReaderTest, ReadGpuInfo_VRAMIsReasonable) {
//     ADLinuxSystemInfoReader reader;
//     ESystemInfo sysInfo = reader.read();

//     // On integrated GPU machines this may be 0 — only check for parsing
//     EXPECT_GE(sysInfo.gpu.vramTotal, 0);
//     EXPECT_GE(sysInfo.gpu.vramUsed, 0);
// }

// TEST(ADLinuxSystemInfoReaderTest, ReadDiskInfo_ModelNameExists) {
//     ADLinuxSystemInfoReader reader;
//     ESystemInfo sysInfo = reader.read();

//     EXPECT_FALSE(sysInfo.disk.model.empty());
//     EXPECT_GT(sysInfo.disk.readSpeed, 0.0);
//     EXPECT_GE(sysInfo.disk.writeSpeed, 0.0); // may be 0 if nothing is writing
// }

// TEST(ADLinuxSystemInfoReaderTest, ReadNetworkInfo_SpeedMayBeZeroButValid) {
//     ADLinuxSystemInfoReader reader;
//     ESystemInfo sysInfo = reader.read();

//     EXPECT_GE(sysInfo.net.rxBytes, 0);
//     EXPECT_GE(sysInfo.net.txBytes, 0);
// }
