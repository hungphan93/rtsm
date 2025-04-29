#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "usecases/ucsysteminforeader.h"
#include "presentation/psystemmonitor.h"

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
    auto systemMonitor = new PSystemMonitor(&useCase, &engine);

    engine.rootContext()->setContextProperty("systemMonitor", systemMonitor);

    engine.loadFromModule("rtsm", "Main");

    return app.exec();
}
