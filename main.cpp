// /// MIT License
// #include <QGuiApplication>
// #include <QQmlApplicationEngine>
// #include <QQmlContext>
// #include <QDebug>
// #include <QIcon>
// #include <QLockFile>
// #include <QDir>
// #include <QWindow>

// #include "ui/qt/system_monitor_view_qt.hpp"
// #include "platform/window_sticky.hpp"
// #include "use_case/system_monitor_interactor.hpp"
// #include "scheduler/system_data_scheduler.hpp"
// #include <csignal>

// // MODIFIED: Use Presenter instead of the old ViewModel
// #include "presenter/system_monitor_presenter.hpp"

// #if defined(__linux__)
// #include "adapter/linux/system_info_reader_linux.hpp"
// #elif defined(_WIN32)
// #include "adapter/window/system_info_reader_window.hpp"
// #elif defined(__APPLE__)
// #include "adapter/mac/system_info_reader_mac.hpp"
// #else
// #error "Unsupported platform"
// #endif

// int main(int argc, char *argv[]) {
//     using namespace std::chrono_literals;

//     /// Ensure only one instance of the application is running at a time
//     // QLockFile lock_file(QDir::tempPath() + "/rtsm.lock");
//     // lock_file.setStaleLockTime(1000);
//     // if (!lock_file.tryLock(100)) {
//     //     qDebug() << "Another instance is already running.";
//     //     return 0;
//     // }

//     QGuiApplication app(argc, argv);
//     app.setWindowIcon(QIcon(":/icons/app_icon.png"));

//     // -----------------------------------------------------------------
//     // STEP 1: INITIALIZE QML ENGINE
//     // -----------------------------------------------------------------
//     QQmlApplicationEngine engine;

//     // -----------------------------------------------------------------
//     // STEP 2: INITIALIZE SYSTEM DATA READER (The Plug-in / Adapter)
//     // -----------------------------------------------------------------
//     // Using Linux specific reader class, implementing the 'system_info_reader'
//     // Interface. Read using C++/STL and system file nodes.
// #if defined(__linux__)
//     adapter::linux2::system_info_reader_linux reader;
// #elif defined(_WIN32)
//     adapter::window::system_info_reader_window reader;
// #elif defined(__APPLE__)
//     adapter::mac::system_info_reader_mac reader;
// #endif

//     // -----------------------------------------------------------------
//     // STEP 3: INITIALIZE PRESENTER & INTERACTOR
//     // -----------------------------------------------------------------
//     // This class receives raw data, automatically converts it to MB/s, %,
//     // and generates formatted Strings for the UI.
//     presenter::system_monitor_presenter presenter;

//     // Attach Presenter to Interactor so the Interactor can pump data out
//     usecase::system_monitor_interactor interactor(reader, presenter);

//     // -----------------------------------------------------------------
//     // STEP 4: INITIALIZE DUMB VIEW (The Humble Object)
//     // -----------------------------------------------------------------
//     // The Qt View is now completely dumb. It only takes the Presenter
//     // and extracts formatted Strings. No logic involved.
//     ui::qt::system_monitor_view_qt monitor_qt(presenter);
//     engine.rootContext()->setContextProperty("system_monitor", &monitor_qt);

//     // -----------------------------------------------------------------
//     // STEP 5: INITIALIZE SCHEDULER (Multi-threading scheduler)
//     // -----------------------------------------------------------------
//     // FIX: Declared AFTER monitor_qt so that when app exits, data_scheduler
//     // is destroyed FIRST, stopping all threads BEFORE monitor_qt is destroyed.
//     scheduler::system_data_scheduler data_scheduler;

//     /// Register periodic data sampling tasks
//     auto _ = data_scheduler.subscribe(300ms, [&interactor]() { interactor.fetch_cpu(); });
//     auto _ = data_scheduler.subscribe(500ms, [&interactor]() { interactor.fetch_memory(); });
//     auto _ = data_scheduler.subscribe(500ms, [&interactor]() { interactor.fetch_gpu(); });
//     auto _ = data_scheduler.subscribe(1000ms, [&interactor]() { interactor.fetch_disk(); });
//     auto _ = data_scheduler.subscribe(1000ms, [&interactor]() { interactor.fetch_net(); });


//     // -----------------------------------------------------------------
//     // STEP 6: CONFIGURE UI (QML & OS Windows)
//     // -----------------------------------------------------------------
//     /// Defer window initialization until the object is created by QML
//     QObject::connect(
//         &engine, &QQmlApplicationEngine::objectCreated,
//         &app, [&](QObject *obj, const QUrl &) {

//             if (!obj) {
//                 qCritical() << "Failed to load QML";
//                 app.exit(-1);
//                 return;
//             }

//             QWindow *window = qobject_cast<QWindow*>(obj);
//             if (!window) return;

//             const QString platform = QGuiApplication::platformName();

//             /// Apply cross-platform sticky window logic
//             platform::make_window_sticky(window, platform);

//             /// Show only after the layer shell has been fully configured
//             window->show();
//         });

//     /// Load QML (DO NOT auto-show)
//     engine.loadFromModule("rtsm", "Main");
//     if (engine.rootObjects().isEmpty()) {
//         return -1;
//     }

//     /// Handle IDE and terminal Stop commands gracefully
//     std::signal(SIGINT, [](int) {
//         qDebug() << "\nReceived SIGINT. Quitting gracefully...";
//         QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
//     });
//     std::signal(SIGTERM, [](int) {
//         qDebug() << "\nReceived SIGTERM. Quitting gracefully...";
//         QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
//     });


//     return app.exec();
// }

#include <vector>
#include <string>
import std;

int main() {

    std::println("Hello, import std!");

    std::vector<int> v = {3, 1, 4, 1, 5, 9};
    std::ranges::sort(v);

    for (auto x : v) {
        std::print("{} ", x);
    }
    std::println("");
//     std::vector<std::string> names = {"C++20", "C++23", "C++26"};
//     std::print("Sử dụng header truyền thống nhưng dùng std::print của C++26!\n");

//     for (const auto& name : names) {
//         std::print("Feature: {}\n", name);
//     }
// #ifdef __cpp_pattern_matching
//     std::print("Máy bạn ĐÃ hỗ trợ inspect!\n");
// #else
//     std::print("Máy bạn CHƯA hỗ trợ inspect. Hãy dùng Lambda thay thế.\n");
// #endif

   // int mode = 1;

    // Gán trực tiếp kết quả vào biến 'status'
    // auto status = inspect (mode) {
    //     0 => "Đang dừng",
    //     1 => "Đang chạy",
    //     2 => "Chế độ chờ",
    //     _ => "Không xác định" // Dấu gạch dưới ở đây đóng vai trò 'default'
    // };

    // std::print("Trạng thái hệ thống: {}\n", status);

}
