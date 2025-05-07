#include "psystemmonitor.h"
#include "entities/esysteminfo.h"
#include "qdebug.h"
#include "usecases/ucsysteminforeader.h"

PSystemMonitor::PSystemMonitor(UCSystemInfoReader* useCase, QObject *parent)
    : QObject{parent},
    m_useCase{useCase}
{
    auto setupTimer = [this](QTimer& timer, int interval, const std::function<void(const ESystemInfo&)>& handler) {
        connect(&timer, &QTimer::timeout, this, [this, handler]() {
            if (m_useCase) {
                const auto info = m_useCase->execute();
                handler(info);
            }
        });
        timer.start(interval);
    };

    setupTimer(m_cpuTimer, 70, [this](const ESystemInfo& info) { updateCpu(info); });
    setupTimer(m_ramGpuTimer, 200, [this](const ESystemInfo& info) { updateRamGpu(info); });
    setupTimer(m_diskNetTimer, 1000, [this](const ESystemInfo& info) { updateDiskNet(info); });
    setupTimer(m_tempTimer, 150, [this](const ESystemInfo& info) { updateTemp(info); });
    qDebug() << "SystemMonitor constructor called:" << this;
}

PSystemMonitor::~PSystemMonitor()
{
    qDebug() << "SystemMonitor destructor called:" << this;
    m_cpuTimer.stop();
    m_ramGpuTimer.stop();
    m_diskNetTimer.stop();
    m_tempTimer.stop();
}

QString PSystemMonitor::cpuModelName() const
{
    qDebug() << "test - " << m_cpuModelName;
    return m_cpuModelName;
}

void PSystemMonitor::setCpuModelName(const QString& newModelName)
{
    if (m_cpuModelName != newModelName)
    {
        m_cpuModelName = newModelName;
        emit cpuModelNameChanged();
    }
}

QString PSystemMonitor::cpuCache() const
{
    return m_cpuCache;
}

void PSystemMonitor::setCpuCache(const QString& newCache)
{
    if (m_cpuCache != newCache)
    {
        m_cpuCache = newCache;
        emit cpuCacheChanged();
    }
}

QString PSystemMonitor::cpuCoreNumber() const
{
    return m_cpuCoreNumber;
}

void PSystemMonitor::setCpuCoreNumber(const QString& newCoreNumber)
{
    if (m_cpuCoreNumber != newCoreNumber)
    {
        m_cpuCoreNumber = newCoreNumber;
        emit cpuCoreNumberChanged();
    }
}

QString PSystemMonitor::cpuPower() const
{
    return m_cpuPower;
}

void PSystemMonitor::setCpuPower(const QString& newPower)
{
    if (m_cpuPower == newPower)
    {
        m_cpuPower = newPower;
        emit cpuPowerChanged();
    }
}

QString PSystemMonitor::cpuTemperatureC() const
{
    return m_cpuTemperatureC;
}

void PSystemMonitor::setCpuTemperatureC(const QString& newTemperatureC)
{
    if (m_cpuTemperatureC != newTemperatureC)
    {
        m_cpuTemperatureC = newTemperatureC;
        emit cpuTemperatureCChanged();
    }
}

QString PSystemMonitor::cpuUsagePercent() const
{
    return m_cpuUsagePercent;
}

void PSystemMonitor::setCpuUsagePercent(const QString& newUsagePercent)
{
    if (m_cpuUsagePercent != newUsagePercent)
    {
        m_cpuUsagePercent = newUsagePercent;
        emit cpuUsagePercentChanged();
    }
}

QString PSystemMonitor::cpuFrequencyMhz() const
{
    return m_cpuFrequencyMhz;
}

void PSystemMonitor::setCpuFrequencyMhz(const QString& newFrequencyMhz)
{
    if (m_cpuFrequencyMhz != newFrequencyMhz)
    {
        m_cpuFrequencyMhz = newFrequencyMhz;
        emit cpuFrequencyMhzChanged();
    }
}

QString PSystemMonitor::memoryPercent()
{
    return m_memoryPercent;
}

int PSystemMonitor::memoryUsed()
{
    return m_memoryUsed;
}

int PSystemMonitor::memoryTotal()
{
    return m_memoryTotal;
}

void PSystemMonitor::setMemoryPercent(const QString &val)
{
    if (m_memoryPercent != val)
    {
        m_memoryPercent = val;
        emit memoryPercentChanged();
    }
}

void PSystemMonitor::setMemoryUsed(const double &val)
{
    if (m_memoryUsed != val)
    {
        m_memoryUsed = val;
        emit memoryUsedChanged();
    }
}

void PSystemMonitor::setMemoryTotal(const double &val)
{
    if (m_memoryTotal != val) {
        m_memoryTotal = val;
        emit memoryTotalChanged();
    }
}

QString PSystemMonitor::gpuName() const
{
    return m_gpuName;
}

QString PSystemMonitor::gpuVramTotal() const
{
    return m_gpuVramTotal;
}

QString PSystemMonitor::gpuVramUsed() const
{
    return m_gpuVramUsed;
}

void PSystemMonitor::setGpuName(const QString& newGpuName)
{
    if (m_gpuName != newGpuName)
    {
        m_gpuName = newGpuName;
        emit gpuNameChanged();
    }
}

void PSystemMonitor::setGpuVramTotal(const QString& newGpuVramTotal)
{
    if (m_gpuVramTotal!= newGpuVramTotal)
    {
        m_gpuVramTotal = newGpuVramTotal;
        emit gpuVramTotalChanged();
    }
}

void PSystemMonitor::setGpuVramUsed(const QString& newGpuVramUsed)
{
    if (m_gpuVramUsed!= newGpuVramUsed)
    {
        m_gpuVramUsed = newGpuVramUsed;
        emit gpuVramUsedChanged();
    }
}

QString PSystemMonitor::netTxBytes() const
{
    return m_netTxBytes;
}

void PSystemMonitor::setNetTxBytes(const QString& newTxBytes)
{
    if (m_netTxBytes != newTxBytes)
    {
        m_netTxBytes = newTxBytes;
        emit netTxBytesChanged();
    }
}

QString PSystemMonitor::netRxBytes() const
{
    return m_netRxBytes;
}

void PSystemMonitor::setNetRxBytes(const QString& newRxBytes)
{
    if (m_netRxBytes != newRxBytes)
    {
        m_netRxBytes = newRxBytes;
        emit netRxBytesChanged();
    }
}

void PSystemMonitor::updateCpu(const ESystemInfo& info) {
    setCpuModelName(QString::fromStdString(info.cpu.modelName));
    setCpuCoreNumber(QString::number(info.cpu.coreNumber));
    setCpuCache(QString::number(info.cpu.cache));
    setCpuPower(QString::number(info.cpu.power));
    if (!info.cpu.threads.empty()) {
        setCpuFrequencyMhz(QString::fromStdString(info.cpu.threads.front().frequencyMhz));
        setCpuUsagePercent(QString::fromStdString(info.cpu.threads.front().usagePercent));
    }
}

void PSystemMonitor::updateRamGpu(const ESystemInfo& info) {
    setMemoryPercent(QString::fromStdString(info.mem.usage_percent));
    setMemoryUsed(info.mem.used_bytes);
    setMemoryTotal(info.mem.total_bytes);
    setGpuName(QString::fromStdString(info.gpu.name));
    setGpuVramTotal(QString::number(info.gpu.vramTotal));
    setGpuVramUsed(QString::number(info.gpu.vramUsed));
}

void PSystemMonitor::updateDiskNet(const ESystemInfo& info) {
    setNetRxBytes(QString::number(info.net.rxBytes));
    setNetTxBytes(QString::number(info.net.txBytes));
}

void PSystemMonitor::updateTemp(const ESystemInfo& info) {
    if (!info.cpu.threads.empty()) {
        setCpuTemperatureC(QString::number(info.cpu.threads.front().temperatureC));
    }
}



