/// MIT License
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QIcon>
#include <QLockFile>
#include <QDir>
#include <QWindow>

#include "ui/qt/system_monitor_qt.hpp"
#include "platform/window_sticky.hpp"

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
    /// Only one the application is open at the time
    QLockFile lock_file(QDir::tempPath() + "/rtsm.lock");
    lock_file.setStaleLockTime(1000);

    if (!lock_file.tryLock(100)) {
        qDebug() << "Another instance is already running.";
        return 0;
    }

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    QQmlApplicationEngine engine;

    /// Construct system info reader
#if defined(__linux__)
    adapter::linux2::system_info_reader_linux reader;
#elif defined(_WIN32)
    adapter::window::system_info_reader_window reader;
#elif defined(__APPLE__)
    adapter::mac::system_info_reader_mac reader;
#endif

    presenter::system_monitor monitor(reader);
    ui::qt::system_monitor_qt monitor_qt(&monitor);
    engine.rootContext()->setContextProperty("system_monitor", &monitor_qt);

    /// Defer window initialization until created
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

            /// Apply sticky behavior (cross-platform)
            platform::make_window_sticky(window, platform);

            /// show only after layer shell configured
            window->show();
        });

    /// Load QML (DO NOT auto-show)
    engine.loadFromModule("rtsm", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
