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

    QQmlApplicationEngine engine;

    // -----------------------------------------------------------------
    // STEP 1: INITIALIZE ADAPTER (Hardware reading core)
    // -----------------------------------------------------------------
#if defined(__linux__)
    adapter::linux2::system_info_reader_linux reader;
#elif defined(_WIN32)
    adapter::window::system_info_reader_window reader;
#elif defined(__APPLE__)
    adapter::mac::system_info_reader_mac reader;
#endif

    // -----------------------------------------------------------------
    // STEP 2: INITIALIZE PRESENTER (Unit/Format Converter)
    // -----------------------------------------------------------------
    // This class receives raw data, automatically converts it to MB/s, %,
    // and generates formatted Strings for the UI.
    presenter::system_monitor_presenter presenter;

    // -----------------------------------------------------------------
    // STEP 3: INITIALIZE USE CASE (Business logic coordinator)
    // -----------------------------------------------------------------
    // Attach Presenter to Interactor so the Interactor can pump data out
    usecase::system_monitor_interactor interactor(presenter);

    // -----------------------------------------------------------------
    // STEP 4: INITIALIZE SCHEDULER (Multi-threading scheduler)
    // -----------------------------------------------------------------
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
    // STEP 5: INITIALIZE DUMB VIEW (The Humble Object)
    // -----------------------------------------------------------------
    // The Qt View is now completely dumb. It only takes the Presenter
    // and extracts formatted Strings. No logic involved.
    ui::qt::system_monitor_view_qt monitor_qt(presenter);
    engine.rootContext()->setContextProperty("system_monitor", &monitor_qt);

    // -----------------------------------------------------------------
    // STEP 6: CONFIGURE UI (QML & OS Windows)
    // -----------------------------------------------------------------
    /// Defer window initialization until the object is created by QML
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [&](QObject *obj, const QUrl &) {
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
        });

    /// Load QML (DO NOT auto-show)
    engine.loadFromModule("rtsm", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
