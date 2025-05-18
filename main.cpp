#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "adapters/adapterfetchmanager.h"
#include "usecases/ucsysteminforeader.h"
#include "presentation/psystemmonitor.h"
#include <memory>

#if defined(__linux__)
#include "adapters/adlinuxsysteminforeader.h"
#elif defined(_WIN32)
#include "adapters/adwindowssysteminforeader.h"
#elif defined(__APPLE__)
#include "adapters/admacsysteminforeader.h"
#else
#error "Unsupported platform"
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

#if defined(__linux__)
    ADLinuxSystemInfoReader reader;
#elif defined(_WIN32)
    ADWindowsSystemInfoReader reader;
#elif defined(__APPLE__)
    ADMacSystemInfoReader reader;
#endif

    UCSystemInfoReader useCase(reader);

    PSystemMonitor systemMonitor(useCase);

    // std::unique_ptr<PSystemMonitor> systemMonitor (new PSystemMonitor(useCase, &engine));

    //engine.rootContext()->setContextProperty("systemMonitor", systemMonitor.get());

    engine.rootContext()->setContextProperty("systemMonitor", &systemMonitor);

    engine.loadFromModule("rtsm", "Main");
    if (engine.rootObjects().isEmpty())
    {
        qCritical() << "Failed to load QML module!";
        return -1;
    }

    return app.exec();
}
