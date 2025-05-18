#include "psystemmonitor.h"
#include "usecases/ucsysteminforeader.h"
#include "adapters/adapterfetchmanager.h"
#include <memory>
#include <qdebug.h>

PSystemMonitor::PSystemMonitor(UCSystemInfoReader& reader, QObject *parent)
    : QObject(parent), m_reader(reader)
{
    m_fetcher = std::make_unique<AdapterFetchManager>([](const ESystemInfo&) {}, m_reader);

    m_fetcher->onCpuUpdate(
        [this](const ESystemInfo& info)
        {
            QMetaObject::invokeMethod(
                this, [this, info]()
                {
                    updateFromInfo(info); // hoặc chỉ update CPU nếu bạn tách nhỏ được
                }, Qt::QueuedConnection);
        });

    m_fetcher->onMemoryGpuUpdate([this](const ESystemInfo& info) {
        QMetaObject::invokeMethod(this, [this, info]() {
            updateFromInfo(info); // hoặc chỉ update RAM + GPU
        }, Qt::QueuedConnection);
    });

    m_fetcher->onDiskNetUpdate([this](const ESystemInfo& info) {
        QMetaObject::invokeMethod(this, [this, info]() {
            updateFromInfo(info); // hoặc chỉ update Disk + Net
        }, Qt::QueuedConnection);
    });

    m_fetcher->onTempUpdate([this](const ESystemInfo& info) {
        QMetaObject::invokeMethod(this, [this, info]() {
            updateFromInfo(info); // hoặc chỉ update Temp
        }, Qt::QueuedConnection);
    });

    m_fetcher->start();
}


PSystemMonitor::~PSystemMonitor() {
    if (m_fetcher)
    {
        m_fetcher->stop();
    }
}

// ========== CPU ==========
QString PSystemMonitor::cpuUsagePercent() const { return m_cpuUsagePercent; }
QString PSystemMonitor::cpuFrequencyMhz() const { return m_cpuFrequencyMhz; }
QString PSystemMonitor::cpuTemperatureC() const { return m_cpuTemperatureC; }
QString PSystemMonitor::cpuCache() const { return m_cpuCache; }
QString PSystemMonitor::cpuCoreNumber() const { return m_cpuCoreNumber; }
QString PSystemMonitor::cpuModelName() const { return m_cpuModelName; }
QString PSystemMonitor::cpuPower() const { return m_cpuPower; }

void PSystemMonitor::setCpuUsagePercent(const QString &v) {
    if (m_cpuUsagePercent != v) {
        m_cpuUsagePercent = v;
        emit cpuUsagePercentChanged();
    }
}
void PSystemMonitor::setCpuFrequencyMhz(const QString &v) {
    if (m_cpuFrequencyMhz != v) {
        m_cpuFrequencyMhz = v;
        emit cpuFrequencyMhzChanged();
    }
}
void PSystemMonitor::setCpuTemperatureC(const QString &v) {
    if (m_cpuTemperatureC != v) {
        m_cpuTemperatureC = v;
        emit cpuTemperatureCChanged();
    }
}
void PSystemMonitor::setCpuCache(const QString &v) {
    if (m_cpuCache != v) {
        m_cpuCache = v;
        emit cpuCacheChanged();
    }
}
void PSystemMonitor::setCpuCoreNumber(const QString &v) {
    if (m_cpuCoreNumber != v) {
        m_cpuCoreNumber = v;
        emit cpuCoreNumberChanged();
    }
}
void PSystemMonitor::setCpuModelName(const QString &v) {
    if (m_cpuModelName != v) {
        m_cpuModelName = v;
        emit cpuModelNameChanged();
    }
}
void PSystemMonitor::setCpuPower(const QString &v) {
    if (m_cpuPower != v) {
        m_cpuPower = v;
        emit cpuPowerChanged();
    }
}

// ========== MEMORY ==========
QString PSystemMonitor::memoryPercent() const { return m_memoryPercent; }
int PSystemMonitor::memoryUsed() const { return m_memoryUsed; }
int PSystemMonitor::memoryTotal() const { return m_memoryTotal; }

void PSystemMonitor::setMemoryPercent(const QString &v) {
    if (m_memoryPercent != v) {
        m_memoryPercent = v;
        emit memoryPercentChanged();
    }
}
void PSystemMonitor::setMemoryUsed(int v) {
    if (m_memoryUsed != v) {
        m_memoryUsed = v;
        emit memoryUsedChanged();
    }
}
void PSystemMonitor::setMemoryTotal(int v) {
    if (m_memoryTotal != v) {
        m_memoryTotal = v;
        emit memoryTotalChanged();
    }
}

// ========== GPU ==========
QString PSystemMonitor::gpuName() const { return m_gpuName; }
QString PSystemMonitor::gpuVramTotal() const { return m_gpuVramTotal; }
QString PSystemMonitor::gpuVramUsed() const { return m_gpuVramUsed; }

void PSystemMonitor::setGpuName(const QString &v) {
    if (m_gpuName != v) {
        m_gpuName = v;
        emit gpuNameChanged();
    }
}
void PSystemMonitor::setGpuVramTotal(const QString &v) {
    if (m_gpuVramTotal != v) {
        m_gpuVramTotal = v;
        emit gpuVramTotalChanged();
    }
}
void PSystemMonitor::setGpuVramUsed(const QString &v) {
    if (m_gpuVramUsed != v) {
        m_gpuVramUsed = v;
        emit gpuVramUsedChanged();
    }
}

// ========== NETWORK ==========
QString PSystemMonitor::netTxBytes() const { return m_netTxBytes; }
QString PSystemMonitor::netRxBytes() const { return m_netRxBytes; }

void PSystemMonitor::setNetTxBytes(const QString &v) {
    if (m_netTxBytes != v) {
        m_netTxBytes = v;
        emit netTxBytesChanged();
    }
}
void PSystemMonitor::setNetRxBytes(const QString &v) {
    if (m_netRxBytes != v) {
        m_netRxBytes = v;
        emit netRxBytesChanged();
    }
}

// ========== DISK ==========
QString PSystemMonitor::diskReadSpeed() const { return m_diskReadSpeed; }
QString PSystemMonitor::diskWriteSpeed() const { return m_diskWriteSpeed; }
QString PSystemMonitor::diskModel() const { return m_diskModel; }

void PSystemMonitor::setDiskReadSpeed(const QString &v) {
    if (m_diskReadSpeed != v) {
        m_diskReadSpeed = v;
        emit diskReadSpeedChanged();
    }
}
void PSystemMonitor::setDiskWriteSpeed(const QString &v) {
    if (m_diskWriteSpeed != v) {
        m_diskWriteSpeed = v;
        emit diskWriteSpeedChanged();
    }
}
void PSystemMonitor::setDiskModel(const QString &v) {
    if (m_diskModel != v) {
        m_diskModel = v;
        emit diskModelChanged();
    }
}

// ========== UPDATE ==========

void PSystemMonitor::updateFromInfo(const ESystemInfo& info)
{
    // CPU
    if (!info.cpu.threads.empty()) {
        const auto& core = info.cpu.threads.front();
        setCpuUsagePercent(QString::fromStdString(core.usagePercent));
        setCpuFrequencyMhz(QString::fromStdString(core.frequencyMhz));
        setCpuTemperatureC(QString::number(core.temperatureC));
        setCpuCache(QString::fromStdString(core.cacheSize));
    }

    qDebug() << "info modelName cpu= " << info.cpu.modelName;
    qDebug() << "info temperatureC cpu= " << info.cpu.threads.front().temperatureC;
    setCpuModelName(QString::fromStdString(info.cpu.modelName));
    setCpuCoreNumber(QString::number(info.cpu.coreNumber));

    // Memory
    setMemoryUsed(static_cast<int>(info.mem.used_bytes));
    setMemoryTotal(static_cast<int>(info.mem.total_bytes));
    setMemoryPercent(QString::fromStdString(info.mem.usage_percent));

    // GPU
    setGpuName(QString::fromStdString(info.gpu.name));
    setGpuVramUsed(QString::number(info.gpu.vramUsed));
    setGpuVramTotal(QString::number(info.gpu.vramTotal));

    // Network
    setNetRxBytes(QString::number(info.net.rxBytes));
    setNetTxBytes(QString::number(info.net.txBytes));

    // Disk
    setDiskModel(QString::fromStdString(info.disk.model));
    setDiskReadSpeed(QString::number(info.disk.readSpeed, 'f', 2));
    setDiskWriteSpeed(QString::number(info.disk.writeSpeed, 'f', 2));
}
