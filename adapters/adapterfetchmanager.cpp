#include "adapterfetchmanager.h"
#include "usecases/ucsysteminforeader.h"
#include "entities/esysteminfo.h"
#include <iostream>

AdapterFetchManager::AdapterFetchManager(Callback callback, UCSystemInfoReader& useCase)
    : m_callback(std::move(callback)), m_useCase(useCase),
    m_cpuTask{std::chrono::milliseconds(80)},
    m_ramGpuTask{std::chrono::milliseconds(500)},
    m_diskNetTask{std::chrono::milliseconds(1000)},
    m_tempTask{std::chrono::milliseconds(300)}
{
}

AdapterFetchManager::~AdapterFetchManager()
{
    stop();
}

void AdapterFetchManager::start()
{
    std::cout << "goi ham start\n";
    m_running = true;

    // Start tasks in separate threads
    m_cpuTask.worker = std::jthread([this]{ run(m_cpuTask); });
    m_ramGpuTask.worker = std::jthread([this] { run(m_ramGpuTask); });
    m_diskNetTask.worker = std::jthread([this] { run(m_diskNetTask); });
    m_tempTask.worker = std::jthread([this] { run(m_tempTask); });
}
void AdapterFetchManager::stop()
{
    m_running = false;

    // Đảm bảo các thread không chạy tiếp
    if (m_cpuTask.worker.joinable()) m_cpuTask.worker.request_stop();
    if (m_ramGpuTask.worker.joinable()) m_ramGpuTask.worker.request_stop();
    if (m_diskNetTask.worker.joinable()) m_diskNetTask.worker.request_stop();
    if (m_tempTask.worker.joinable()) m_tempTask.worker.request_stop();
}

void AdapterFetchManager::run(Task& task)
{
    while (m_running)
    {
        std::cout << "goi ham run1\n";
        ESystemInfo info = m_useCase.execute();
        {
            std::cout << "goi ham run2\n";
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            if (task.callback)
                task.callback(info);
            std::cout << "goi ham run3\n";
        }

        std::this_thread::sleep_for(task.interval);
    }
}

void AdapterFetchManager::onCpuUpdate(Callback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_cpuTask.callback = std::move(cb);
    std::cout << "goi ham onCpuUpdate\n";
}

void AdapterFetchManager::onMemoryGpuUpdate(Callback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_ramGpuTask.callback = std::move(cb);
    std::cout << "goi ham onMemoryGpuUpdate\n";
}

void AdapterFetchManager::onDiskNetUpdate(Callback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_diskNetTask.callback = std::move(cb);
}

void AdapterFetchManager::onTempUpdate(Callback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_tempTask.callback = std::move(cb);
}
