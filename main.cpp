#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

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
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

#if defined(__linux__)
    adapter::linux2::system_info_reader_linux reader;
#elif defined(_WIN32)
    adapter::window::system_info_reader_window reader;
#elif defined(__APPLE__)
    adapter::mac::system_info_reader_mac reader;
#endif

    //  UCSystemInfoReader useCase(reader);
    //   usecase::system_info_reader use_case(reader);

    //PSystemMonitor systemMonitor(useCase);

    // std::unique_ptr<PSystemMonitor> systemMonitor (new PSystemMonitor(useCase, &engine));

    //engine.rootContext()->setContextProperty("systemMonitor", systemMonitor.get());

    //engine.rootContext()->setContextProperty("systemMonitor", &systemMonitor);
    engine.loadFromModule("rtsm", "Main");
 //   engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML module!";
        return -1;
    }

    return app.exec();
}
