/// MIT License
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QIcon>
#include <QLockFile>
#include <QDir>
#include <QWindow>
#include <memory>

#include "./ui/qt/system_monitor_view_qt.hpp"
#include "platform/window_sticky.hpp"
#include <csignal>

#include "ui/qt/system_monitor_backend_fwd.hpp"

import adapter;
import system_data_scheduler;
import usecase;
import presenter;

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

    adapter::linux2::system_info_reader_linux reader;

    auto presenter = std::make_shared<presenter::system_monitor_presenter>();

    usecase::system_monitor_interactor interactor(reader, *presenter);


    auto backend = create_system_monitor_backend(presenter);
    ui::qt::system_monitor_view_qt monitor_qt(backend);

    engine.rootContext()->setContextProperty("system_monitor", &monitor_qt);

    scheduler::system_data_scheduler data_scheduler;

    /// Register periodic data sampling tasks
    [[maybe_unused]] auto t1_ = data_scheduler.subscribe(300ms, [&interactor]() { interactor.fetch_cpu(); });
    [[maybe_unused]] auto t2_ = data_scheduler.subscribe(500ms, [&interactor]() { interactor.fetch_memory(); });
    [[maybe_unused]] auto t3_ = data_scheduler.subscribe(500ms, [&interactor]() { interactor.fetch_gpu(); });
    [[maybe_unused]] auto t4_ = data_scheduler.subscribe(1000ms, [&interactor]() { interactor.fetch_disk(); });
    [[maybe_unused]] auto t5_ = data_scheduler.subscribe(1000ms, [&interactor]() { interactor.fetch_net(); });


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
    if (engine.rootObjects().isEmpty()) {
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


    return app.exec();
}
