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
    QLockFile lock_file(QDir::tempPath() + "/rtsm.lock");
    lock_file.setStaleLockTime(1000); // 1 second

    if (!lock_file.tryLock(100)) {
        qDebug() << "Another instance is already running. Exiting bye bye.";
        return 0;
    }

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

/// Construct platform-appropriate system info reader
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
    engine.loadFromModule("rtsm", "Main");

    const auto& roots = engine.rootObjects();
    if (roots.isEmpty()) {
        qCritical() << "Failed to load QML module!";
        return -1;
    }

    QPointer<QWindow> window = qobject_cast<QWindow*>(roots.first());
    if (window) {
        platform::make_window_sticky(window);
    }

    return app.exec();
}
