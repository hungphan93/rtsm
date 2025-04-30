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
    Q_PROPERTY(QString usagePercent READ usagePercent WRITE setUsagePercent NOTIFY usagePercentChanged)
    Q_PROPERTY(QString frequencyMhz READ frequencyMhz WRITE setFrequencyMhz NOTIFY frequencyMhzChanged)
    Q_PROPERTY(QString temperatureC READ temperatureC WRITE setTemperatureC NOTIFY temperatureCChanged)
    Q_PROPERTY(int cache READ cache WRITE setCache NOTIFY cacheChanged)
    Q_PROPERTY(int coreNumber READ coreNumber WRITE setCoreNumber NOTIFY coreNumberChanged)
    Q_PROPERTY(QString modelName READ modelName WRITE setModelName NOTIFY modelNameChanged)
    Q_PROPERTY(double power READ power WRITE setPower NOTIFY powerChanged)

    //memory
    Q_PROPERTY(QString memoryPercent READ memoryPercent WRITE setMemoryPercent NOTIFY memoryPercentChanged)
    Q_PROPERTY(int memoryUsed READ memoryUsed WRITE setMemoryUsed NOTIFY memoryUsedChanged)
    Q_PROPERTY(int memoryTotal READ memoryTotal WRITE setMemoryTotal NOTIFY memoryTotalChanged)

    //gpu
    Q_PROPERTY(QString gpuName READ gpuName WRITE setGpuName NOTIFY gpuNameChanged)
    Q_PROPERTY(double gpuVramTotal READ gpuVramTotal WRITE setGpuVramTotal NOTIFY gpuVramTotalChanged)
    Q_PROPERTY(double gpuVramUsed READ gpuVramUsed WRITE setGpuVramUsed NOTIFY gpuVramUsedChanged)

public:
    explicit PSystemMonitor(UCSystemInfoReader* useCase = nullptr, QObject *parent = nullptr);
    ~PSystemMonitor();

    //CPU getter
    QString modelName() const;
    int cache() const;
    double power() const;
    int coreNumber() const;
    QString temperatureC() const;
    QString usagePercent() const;
    QString frequencyMhz() const;

    //CPU setter
    void setModelName(const QString newModelName);
    void setCache(int newCache);
    void setCoreNumber(int newCoreNumber);
    void setPower(double newPower);
    void setTemperatureC(const QString newTemperatureC);
    void setUsagePercent(const QString newUsagePercent);
    void setFrequencyMhz(const QString newFrequencyMhz);

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
    double gpuVramTotal() const;
    double gpuVramUsed() const;

    //gpu setter
    void setGpuName(const QString& newGpuName);
    void setGpuVramTotal(const double& newGpuVramTotal);
    void setGpuVramUsed(const double& newGpuVramUsed);

signals:
    //CPU
    void modelNameChanged();
    void cacheChanged();
    void coreNumberChanged();
    void powerChanged();
    void temperatureCChanged();
    void usagePercentChanged();
    void frequencyMhzChanged();

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
    QString m_usagePercent;
    QString m_frequencyMhz;
    QString m_temperatureC;
    int m_cache;
    int m_coreNumber;
    double m_power;
    QString m_modelName;
    QTimer timer;
    QString m_memoryPercent;
    double m_memoryUsed;
    double m_memoryTotal;
    QString m_gpuName;
    double m_gpuVramTotal;
    double m_gpuVramUsed;
};

#endif // PSYSTEMMONITOR_H

