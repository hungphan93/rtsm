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

QString PSystemMonitor::modelName() const
{
    qDebug() << "test - " << m_modelName;
    return m_modelName;
}

void PSystemMonitor::setModelName(const QString newModelName)
{
    if (m_modelName == newModelName)
        return;
    m_modelName = newModelName;
    emit modelNameChanged();
}

int PSystemMonitor::cache() const
{
    return m_cache;
}

void PSystemMonitor::setCache(int newCache)
{
    if (m_cache == newCache)
        return;
    m_cache = newCache;
    emit cacheChanged();
}

int PSystemMonitor::coreNumber() const
{
    return m_coreNumber;
}

void PSystemMonitor::setCoreNumber(int newCoreNumber)
{
    if (m_coreNumber == newCoreNumber)
        return;
    m_coreNumber = newCoreNumber;
    emit coreNumberChanged();
}

double PSystemMonitor::power() const
{
    return m_power;
}

void PSystemMonitor::setPower(double newPower)
{
    if (m_power == newPower)
        return;
    m_power = newPower;
    emit powerChanged();
}
void PSystemMonitor::updateSystemInfo()
{
    qDebug() << "\nCalling updateSystemInfo funtion\n" << this;
    if (!m_useCase) return;

    ESystemInfo info = m_useCase->execute();

    setModelName(QString::fromStdString(info.cpu.modelName));
    setCoreNumber(info.cpu.coreNumber);
    setCache(info.cpu.cache);
    setPower(info.cpu.power);
    setFrequencyMhz(QString::fromStdString(info.cpu.threads.front().frequencyMhz));
    setUsagePercent(QString::fromStdString(info.cpu.threads.front().usagePercent));
    setTemperatureC(QString::fromStdString(info.cpu.threads.front().temperatureC));

    //memory
    setMemoryPercent(QString::fromStdString(info.mem.usage_percent));
    setMemoryUsed(info.mem.used_bytes);
    setMemoryTotal(info.mem.total_bytes);


}

QString PSystemMonitor::temperatureC() const
{
    return m_temperatureC;
}

void PSystemMonitor::setTemperatureC(const QString newTemperatureC)
{
    if (m_temperatureC == newTemperatureC)
        return;
    m_temperatureC = newTemperatureC;
    emit temperatureCChanged();
}

QString PSystemMonitor::usagePercent() const
{
    return m_usagePercent;
}

void PSystemMonitor::setUsagePercent(const QString newUsagePercent)
{
    if (m_usagePercent == newUsagePercent)
        return;
    m_usagePercent = newUsagePercent;
    emit usagePercentChanged();
}

QString PSystemMonitor::frequencyMhz() const
{
    return m_frequencyMhz;
}

void PSystemMonitor::setFrequencyMhz(const QString newFrequencyMhz)
{
    if (m_frequencyMhz == newFrequencyMhz)
        return;
    m_frequencyMhz = newFrequencyMhz;
    emit frequencyMhzChanged();
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

double PSystemMonitor::gpuVramTotal() const
{
    return m_gpuVramTotal;
}

double PSystemMonitor::gpuVramUsed() const
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

void PSystemMonitor::setGpuVramTotal(const double& newGpuVramTotal)
{
    if (m_gpuVramTotal!= newGpuVramTotal)
    {
        m_gpuVramTotal = newGpuVramTotal;
        emit gpuVramTotalChanged();
    }
}

void PSystemMonitor::setGpuVramUsed(const double& newGpuVramUsed)
{
    if (m_gpuVramUsed!= newGpuVramUsed)
    {
        m_gpuVramUsed = newGpuVramUsed;
        emit gpuVramUsedChanged();
    }
}
