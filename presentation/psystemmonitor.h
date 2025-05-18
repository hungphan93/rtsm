#ifndef PSYSTEMMONITOR_H
#define PSYSTEMMONITOR_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>
#include "entities/esysteminfo.h"

class AdapterFetchManager;
class UCSystemInfoReader;

class PSystemMonitor : public QObject
{
    Q_OBJECT

    // CPU
    Q_PROPERTY(QString cpuUsagePercent READ cpuUsagePercent WRITE setCpuUsagePercent NOTIFY cpuUsagePercentChanged)
    Q_PROPERTY(QString cpuFrequencyMhz READ cpuFrequencyMhz WRITE setCpuFrequencyMhz NOTIFY cpuFrequencyMhzChanged)
    Q_PROPERTY(QString cpuTemperatureC READ cpuTemperatureC WRITE setCpuTemperatureC NOTIFY cpuTemperatureCChanged)
    Q_PROPERTY(QString cpuCache READ cpuCache WRITE setCpuCache NOTIFY cpuCacheChanged)
    Q_PROPERTY(QString cpuCoreNumber READ cpuCoreNumber WRITE setCpuCoreNumber NOTIFY cpuCoreNumberChanged)
    Q_PROPERTY(QString cpuModelName READ cpuModelName WRITE setCpuModelName NOTIFY cpuModelNameChanged)
    Q_PROPERTY(QString cpuPower READ cpuPower WRITE setCpuPower NOTIFY cpuPowerChanged)

    // Memory
    Q_PROPERTY(QString memoryPercent READ memoryPercent WRITE setMemoryPercent NOTIFY memoryPercentChanged)
    Q_PROPERTY(int memoryUsed READ memoryUsed WRITE setMemoryUsed NOTIFY memoryUsedChanged)
    Q_PROPERTY(int memoryTotal READ memoryTotal WRITE setMemoryTotal NOTIFY memoryTotalChanged)

    // GPU
    Q_PROPERTY(QString gpuName READ gpuName WRITE setGpuName NOTIFY gpuNameChanged)
    Q_PROPERTY(QString gpuVramTotal READ gpuVramTotal WRITE setGpuVramTotal NOTIFY gpuVramTotalChanged)
    Q_PROPERTY(QString gpuVramUsed READ gpuVramUsed WRITE setGpuVramUsed NOTIFY gpuVramUsedChanged)

    // Network
    Q_PROPERTY(QString netTxBytes READ netTxBytes WRITE setNetTxBytes NOTIFY netTxBytesChanged)
    Q_PROPERTY(QString netRxBytes READ netRxBytes WRITE setNetRxBytes NOTIFY netRxBytesChanged)

    // Disk
    Q_PROPERTY(QString diskReadSpeed READ diskReadSpeed WRITE setDiskReadSpeed NOTIFY diskReadSpeedChanged)
    Q_PROPERTY(QString diskWriteSpeed READ diskWriteSpeed WRITE setDiskWriteSpeed NOTIFY diskWriteSpeedChanged)
    Q_PROPERTY(QString diskModel READ diskModel WRITE setDiskModel NOTIFY diskModelChanged)

public:
    explicit PSystemMonitor(UCSystemInfoReader& reader, QObject *parent = nullptr);
    ~PSystemMonitor();

    // CPU
    QString cpuUsagePercent() const;
    QString cpuFrequencyMhz() const;
    QString cpuTemperatureC() const;
    QString cpuCache() const;
    QString cpuCoreNumber() const;
    QString cpuModelName() const;
    QString cpuPower() const;

    void setCpuUsagePercent(const QString &);
    void setCpuFrequencyMhz(const QString &);
    void setCpuTemperatureC(const QString &);
    void setCpuCache(const QString &);
    void setCpuCoreNumber(const QString &);
    void setCpuModelName(const QString &);
    void setCpuPower(const QString &);

    // Memory
    QString memoryPercent() const;
    int memoryUsed() const;
    int memoryTotal() const;

    void setMemoryPercent(const QString &);
    void setMemoryUsed(int);
    void setMemoryTotal(int);

    // GPU
    QString gpuName() const;
    QString gpuVramTotal() const;
    QString gpuVramUsed() const;

    void setGpuName(const QString &);
    void setGpuVramTotal(const QString &);
    void setGpuVramUsed(const QString &);

    // Network
    QString netTxBytes() const;
    QString netRxBytes() const;

    void setNetTxBytes(const QString &);
    void setNetRxBytes(const QString &);

    // Disk
    QString diskReadSpeed() const;
    QString diskWriteSpeed() const;
    QString diskModel() const;

    void setDiskReadSpeed(const QString &);
    void setDiskWriteSpeed(const QString &);
    void setDiskModel(const QString &);

signals:
    // CPU
    void cpuUsagePercentChanged();
    void cpuFrequencyMhzChanged();
    void cpuTemperatureCChanged();
    void cpuCacheChanged();
    void cpuCoreNumberChanged();
    void cpuModelNameChanged();
    void cpuPowerChanged();

    // Memory
    void memoryPercentChanged();
    void memoryUsedChanged();
    void memoryTotalChanged();

    // GPU
    void gpuNameChanged();
    void gpuVramTotalChanged();
    void gpuVramUsedChanged();

    // Network
    void netTxBytesChanged();
    void netRxBytesChanged();

    // Disk
    void diskReadSpeedChanged();
    void diskWriteSpeedChanged();
    void diskModelChanged();

private:
    void updateFromInfo(const ESystemInfo& info);

    UCSystemInfoReader& m_reader;
    std::unique_ptr<AdapterFetchManager> m_fetcher;

    // CPU
    QString m_cpuUsagePercent;
    QString m_cpuFrequencyMhz;
    QString m_cpuTemperatureC;
    QString m_cpuCache;
    QString m_cpuCoreNumber;
    QString m_cpuModelName;
    QString m_cpuPower;

    // Memory
    QString m_memoryPercent;
    int m_memoryUsed = 0;
    int m_memoryTotal = 0;

    // GPU
    QString m_gpuName;
    QString m_gpuVramTotal;
    QString m_gpuVramUsed;

    // Network
    QString m_netTxBytes;
    QString m_netRxBytes;

    // Disk
    QString m_diskReadSpeed;
    QString m_diskWriteSpeed;
    QString m_diskModel;
};

#endif // PSYSTEMMONITOR_H
