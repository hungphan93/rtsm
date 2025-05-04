#ifndef PSYSTEMMONITOR_H
#define PSYSTEMMONITOR_H
#include <QObject>
#include <QString>
#include <QVector>
#include <QTimer>

class UCSystemInfoReader;

class PSystemMonitor: public QObject
{
    Q_OBJECT
    //CPU
    Q_PROPERTY(QString cpuUsagePercent READ cpuUsagePercent WRITE setCpuUsagePercent NOTIFY cpuUsagePercentChanged)
    Q_PROPERTY(QString cpuFrequencyMhz READ cpuFrequencyMhz WRITE setCpuFrequencyMhz NOTIFY cpuFrequencyMhzChanged)
    Q_PROPERTY(QString cpuTemperatureC READ cpuTemperatureC WRITE setCpuTemperatureC NOTIFY cpuTemperatureCChanged)
    Q_PROPERTY(QString cpuCache READ cpuCache WRITE setCpuCache NOTIFY cpuCacheChanged)
    Q_PROPERTY(QString cpuCoreNumber READ cpuCoreNumber WRITE setCpuCoreNumber NOTIFY cpuCoreNumberChanged)
    Q_PROPERTY(QString cpuModelName READ cpuModelName WRITE setCpuModelName NOTIFY cpuModelNameChanged)
    Q_PROPERTY(QString cpuPower READ cpuPower WRITE setCpuPower NOTIFY cpuPowerChanged)

    //memory
    Q_PROPERTY(QString memoryPercent READ memoryPercent WRITE setMemoryPercent NOTIFY memoryPercentChanged)
    Q_PROPERTY(int memoryUsed READ memoryUsed WRITE setMemoryUsed NOTIFY memoryUsedChanged)
    Q_PROPERTY(int memoryTotal READ memoryTotal WRITE setMemoryTotal NOTIFY memoryTotalChanged)

    //gpu
    Q_PROPERTY(QString gpuName READ gpuName WRITE setGpuName NOTIFY gpuNameChanged)
    Q_PROPERTY(QString gpuVramTotal READ gpuVramTotal WRITE setGpuVramTotal NOTIFY gpuVramTotalChanged)
    Q_PROPERTY(QString gpuVramUsed READ gpuVramUsed WRITE setGpuVramUsed NOTIFY gpuVramUsedChanged)

public:
    explicit PSystemMonitor(UCSystemInfoReader* useCase = nullptr, QObject *parent = nullptr);
    ~PSystemMonitor();

    //CPU getter
    QString cpuModelName() const;
    QString cpuCache() const;
    QString cpuPower() const;
    QString cpuCoreNumber() const;
    QString cpuTemperatureC() const;
    QString cpuUsagePercent() const;
    QString cpuFrequencyMhz() const;

    //CPU setter
    void setCpuModelName(const QString& newModelName);
    void setCpuCache(const QString& newCache);
    void setCpuCoreNumber(const QString& newCoreNumber);
    void setCpuPower(const QString& newPower);
    void setCpuTemperatureC(const QString& newTemperatureC);
    void setCpuUsagePercent(const QString& newUsagePercent);
    void setCpuFrequencyMhz(const QString& newFrequencyMhz);

    //memory get
    QString memoryPercent();
    int memoryUsed();
    int memoryTotal();

    //memory set
    void setMemoryPercent(const QString& newMemoryPercent);
    void setMemoryUsed(const double& setMemoryUsed);
    void setMemoryTotal(const double& setMemoryTotal);

    //gpu get
    QString gpuName() const;
    QString gpuVramTotal() const;
    QString gpuVramUsed() const;

    //gpu setter
    void setGpuName(const QString& newGpuName);
    void setGpuVramTotal(const QString& newGpuVramTotal);
    void setGpuVramUsed(const QString& newGpuVramUsed);

signals:
    //CPU
    void cpuModelNameChanged();
    void cpuCacheChanged();
    void cpuCoreNumberChanged();
    void cpuPowerChanged();
    void cpuTemperatureCChanged();
    void cpuUsagePercentChanged();
    void cpuFrequencyMhzChanged();

    //memory
    void memoryPercentChanged();
    void memoryUsedChanged();
    void memoryTotalChanged();

    //gpu
    void gpuNameChanged();
    void gpuVramTotalChanged();
    void gpuVramUsedChanged();

private:
    void updateSystemInfo();
    UCSystemInfoReader* m_useCase;
    //cpu
    QString m_cpuUsagePercent;
    QString m_cpuFrequencyMhz;
    QString m_cpuTemperatureC;
    QString m_cpuCache;
    QString m_cpuCoreNumber;
    QString m_cpuPower;
    QString m_cpuModelName;

    QTimer timer;
    //memory
    QString m_memoryPercent;
    double m_memoryUsed;
    double m_memoryTotal;
    //gpu
    QString m_gpuName;
    QString m_gpuVramTotal;
    QString m_gpuVramUsed;
};

#endif // PSYSTEMMONITOR_H

