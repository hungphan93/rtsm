/// MIT License
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QIcon>
#include <QLockFile>
#include <QDir>
#include <QWindow>

#include "ui/qt/system_monitor_view_qt.hpp"
#include "platform/window_sticky.hpp"
#include "use_case/system_monitor_interactor.hpp"
#include "scheduler/system_data_scheduler.hpp"
#include <csignal>

// MODIFIED: Use Presenter instead of the old ViewModel
#include "presenter/system_monitor_presenter.hpp"

#if defined(__linux__)
#include "adapter/linux/system_info_reader_linux.hpp"
#elif defined(_WIN32)
#include "adapter/window/system_info_reader_window.hpp"
#elif defined(__APPLE__)
#include "adapter/mac/system_info_reader_mac.hpp"
#else
#error "Unsupported platform"
#endif

int main(int argc, char *argv[]) {
    using namespace std::chrono_literals;

    /// Ensure only one instance of the application is running at a time
    // QLockFile lock_file(QDir::tempPath() + "/rtsm.lock");
    // lock_file.setStaleLockTime(1000);
    // if (!lock_file.tryLock(100)) {
    //     qDebug() << "Another instance is already running.";
    //     return 0;
    // }

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    // -----------------------------------------------------------------
    // STEP 1: INITIALIZE QML ENGINE
    // -----------------------------------------------------------------
    QQmlApplicationEngine engine;

    // -----------------------------------------------------------------
    // STEP 2: INITIALIZE SYSTEM DATA READER (The Plug-in / Adapter)
    // -----------------------------------------------------------------
    // Using Linux specific reader class, implementing the 'system_info_reader'
    // Interface. Read using C++/STL and system file nodes.
#if defined(__linux__)
    adapter::linux2::system_info_reader_linux reader;
#elif defined(_WIN32)
    adapter::window::system_info_reader_window reader;
#elif defined(__APPLE__)
    adapter::mac::system_info_reader_mac reader;
#endif

    // -----------------------------------------------------------------
    // STEP 3: INITIALIZE PRESENTER & INTERACTOR
    // -----------------------------------------------------------------
    // This class receives raw data, automatically converts it to MB/s, %,
    // and generates formatted Strings for the UI.
    presenter::system_monitor_presenter presenter;

    // Attach Presenter to Interactor so the Interactor can pump data out
    usecase::system_monitor_interactor interactor(presenter);

    // -----------------------------------------------------------------
    // STEP 4: INITIALIZE DUMB VIEW (The Humble Object)
    // -----------------------------------------------------------------
    // The Qt View is now completely dumb. It only takes the Presenter
    // and extracts formatted Strings. No logic involved.
    ui::qt::system_monitor_view_qt monitor_qt(presenter);
    engine.rootContext()->setContextProperty("system_monitor", &monitor_qt);

    // -----------------------------------------------------------------
    // STEP 5: INITIALIZE SCHEDULER (Multi-threading scheduler)
    // -----------------------------------------------------------------
    // FIX: Declared AFTER monitor_qt so that when app exits, data_scheduler 
    // is destroyed FIRST, stopping all threads BEFORE monitor_qt is destroyed.
    scheduler::system_data_scheduler data_scheduler(reader);

    /// Register periodic data sampling tasks
    auto cpu_id = data_scheduler.subscribe(300ms,
                                           &usecase::system_info_reader::read_cpu,
                                           [&interactor](const entity::cpu& cpu) {
                                               interactor.on_cpu_updated(cpu);
                                           });

    auto memory_id = data_scheduler.subscribe(500ms,
                                              &usecase::system_info_reader::read_memory,
                                              [&interactor](const entity::memory& memory) {
                                                  interactor.on_memory_updated(memory);
                                              });

    auto gpu_id = data_scheduler.subscribe(500ms,
                                           &usecase::system_info_reader::read_gpu,
                                           [&interactor](const entity::gpu& gpu) {
                                               interactor.on_gpu_updated(gpu);
                                           });

    auto disk_id = data_scheduler.subscribe(1000ms,
                                            &usecase::system_info_reader::read_disk,
                                            [&interactor](const entity::disk& disk) {
                                                interactor.on_disk_updated(disk);
                                            });

    auto net_id = data_scheduler.subscribe(1000ms,
                                           &usecase::system_info_reader::read_net,
                                           [&interactor](const entity::net& net) {
                                               interactor.on_net_updated(net);
                                           });

    // -----------------------------------------------------------------
    // STEP 6: CONFIGURE UI (QML & OS Windows)
    // -----------------------------------------------------------------
    /// Defer window initialization until the object is created by QML
    qDebug ("\n 000thoat app hung phan\n");
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [&](QObject *obj, const QUrl &) {
            qDebug ("\n 111thoat app hung phan\n");

            if (!obj) {
                qCritical() << "Failed to load QML";
                app.exit(-1);
                return;
            }

            QWindow *window = qobject_cast<QWindow*>(obj);
            if (!window) return;

            const QString platform = QGuiApplication::platformName();

            /// Apply cross-platform sticky window logic
            platform::make_window_sticky(window, platform);

            /// Show only after the layer shell has been fully configured
            window->show();
            qDebug ("\n 222thoat app hung phan\n");
        });

    /// Load QML (DO NOT auto-show)
    engine.loadFromModule("rtsm", "Main");
    if (engine.rootObjects().isEmpty()) {
        qDebug ("\n 3333thoat app hung phan\n");
        return -1;
    }

    /// Handle IDE and terminal Stop commands gracefully
    std::signal(SIGINT, [](int) {
        qDebug() << "\nReceived SIGINT. Quitting gracefully...";
        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
    });
    std::signal(SIGTERM, [](int) {
        qDebug() << "\nReceived SIGTERM. Quitting gracefully...";
        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
    });

    qDebug() << "444hung phan - starting event loop";
    int ret = app.exec();
    
    qDebug() << "555 - event loop finished. Returning from main()...";
    return ret;
}
