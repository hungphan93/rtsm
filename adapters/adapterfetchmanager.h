#ifndef ADAPTERFETCHMANAGER_H
#define ADAPTERFETCHMANAGER_H

#include <functional>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

struct ESystemInfo;
struct UCSystemInfoReader;

class AdapterFetchManager
{
public:
    using Callback = std::function<void(const ESystemInfo&)>;

    explicit AdapterFetchManager(Callback callback, UCSystemInfoReader& useCase);
    ~AdapterFetchManager();

    void start();
    void stop();

    void onCpuUpdate(Callback cb);
    void onMemoryGpuUpdate(Callback cb);
    void onDiskNetUpdate(Callback cb);
    void onTempUpdate(Callback cb);

private:
    struct Task
    {
        std::chrono::milliseconds interval;
        Callback callback;
        std::jthread worker;
    };

    void run(Task& task);

    Callback m_callback;
    UCSystemInfoReader& m_useCase;
    std::atomic<bool> m_running{false};

    Task m_cpuTask;
    Task m_ramGpuTask;
    Task m_diskNetTask;
    Task m_tempTask;

    std::mutex m_callbackMutex;
};

#endif // ADAPTERFETCHMANAGER_H
