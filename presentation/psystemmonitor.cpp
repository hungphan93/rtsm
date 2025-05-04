#include "psystemmonitor.h"
#include "entities/esysteminfo.h"
#include "qdebug.h"
#include "usecases/ucsysteminforeader.h"

PSystemMonitor::PSystemMonitor(UCSystemInfoReader* useCase, QObject *parent)
    : QObject{parent},
    m_useCase{useCase}
{
    connect(&timer, &QTimer::timeout, this, &PSystemMonitor::updateSystemInfo);
    timer.start(100);
    qDebug() << "SystemMonitor constructor called:" << this;
    updateSystemInfo();
}

PSystemMonitor::~PSystemMonitor()
{
    qDebug() << "SystemMonitor destructor called:" << this;
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
    if (m_cpuCache == newCache)
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
void PSystemMonitor::updateSystemInfo()
{
    qDebug() << "\nCalling updateSystemInfo funtion\n" << this;
    if (!m_useCase) return;

    ESystemInfo info = m_useCase->execute();

    setCpuModelName(QString::fromStdString(info.cpu.modelName));
    setCpuCoreNumber(QString::number(info.cpu.coreNumber));
    setCpuCache(QString::number(info.cpu.cache));
    setCpuPower(QString::number(info.cpu.power));
    setCpuFrequencyMhz(QString::fromStdString(info.cpu.threads.front().frequencyMhz));
    setCpuUsagePercent(QString::fromStdString(info.cpu.threads.front().usagePercent));
    setCpuTemperatureC(QString::number(info.cpu.threads.front().temperatureC));

    //memory
    setMemoryPercent(QString::fromStdString(info.mem.usage_percent));
    setMemoryUsed(info.mem.used_bytes);
    setMemoryTotal(info.mem.total_bytes);

    //gpu
    setGpuName(QString::fromStdString(info.gpu.name));
    setGpuVramTotal(QString::number(info.gpu.vramTotal));
    setGpuVramUsed(QString::number(info.gpu.vramUsed));


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
