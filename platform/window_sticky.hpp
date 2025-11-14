/// MIT License
#ifndef PLATFORM_WINDOW_STICKY_HPP
#define PLATFORM_WINDOW_STICKY_HPP

#include <QWindow>
#include <QPointer>
#include <QString>
#include <iostream>

#if defined(__linux__)
/// Wayland LayerShell
// #include <LayerShellQt/Shell>
// #include <LayerShellQt/Window>
/// X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#elif defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <objc/objc-runtime.h>
#endif

namespace platform {

inline void make_window_sticky(QPointer<QWindow> window = nullptr, QString platform_name = QString()) {
    if (!window) {
        std::cerr << "[platform] No window provided for make_window_sticky.\n";
        return;
    }

#if defined(__linux__)
    if (platform_name.startsWith("wayland", Qt::CaseInsensitive)) {
        std::clog << "[Wayland] Currently LayerShell don'nt supported on wayland\n";
        // std::clog << "[Wayland] Applying LayerShell\n";

        // LayerShellQt::Shell::useLayerShell();
        // auto *ls = LayerShellQt::Window::get(window);

        // ls->setLayer(LayerShellQt::Window::LayerBottom);
        // ls->setExclusiveZone(0);       // no reserved space
        // ls->setMargins(QMargins(0,0,0,0));    // no gaps
        // ls->setScope("rtsm-monitor-bar");
        // ls->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);

        // ls->setAnchors(
        //     LayerShellQt::Window::Anchors(
        //         LayerShellQt::Window::AnchorTop
        //         | LayerShellQt::Window::AnchorLeft
        //         | LayerShellQt::Window::AnchorRight
        //         )
        //     );
        // std::clog << "[platform] Wayland sticky applied (background layer + click-through)\n";
        return;
    }

    else if (platform_name.startsWith("xcb", Qt::CaseInsensitive) ||
             platform_name.contains("x11", Qt::CaseInsensitive)) {
        Display* display = XOpenDisplay(nullptr);
        if (!display) {
            std::cerr << "[platform] Failed to open X11 display.\n";
            return;
        }

        const Window win_id = window->winId();

        /// Helper to intern atom
        auto get_atom = [&](const char* name) -> Atom {
            Atom atom = XInternAtom(display, name, False);
            if (atom == None) {
                std::clog << "[platform] Failed to get X11 atom:" << name << "\n";
            }
            return atom;
        };

        /// _NET_WM_DESKTOP hint (appear on all desktops)
        Atom desktop_atom = get_atom("_NET_WM_DESKTOP");
        if (desktop_atom != None) {
            constexpr unsigned long ALL_DESKTOPS = 0xFFFFFFFF;
            XChangeProperty(display, win_id, desktop_atom, XA_CARDINAL, 32, PropModeReplace,
                            reinterpret_cast<unsigned char*>(const_cast<unsigned long*>(&ALL_DESKTOPS)), 1);
            std::clog << "[platform] Set _NET_WM_DESKTOP to all desktops.\n";
        }

        /// Proper client message for _NET_WM_STATE sticky + below
        Atom wm_state = get_atom("_NET_WM_STATE");
        Atom sticky   = get_atom("_NET_WM_STATE_STICKY");
        Atom below    = get_atom("_NET_WM_STATE_BELOW");

        if (wm_state && sticky && below) {
            XEvent e;
            std::memset(&e, 0, sizeof(e));
            e.xclient.type = ClientMessage;
            e.xclient.message_type = wm_state;
            e.xclient.display = display;
            e.xclient.window = win_id;
            e.xclient.format = 32;
            e.xclient.data.l[0] = 1; /// _NET_WM_STATE_ADD
            e.xclient.data.l[1] = sticky;
            e.xclient.data.l[2] = below;

            XSendEvent(display, DefaultRootWindow(display), False,
                       SubstructureRedirectMask | SubstructureNotifyMask, &e);

            std::clog << "[platform] Sent _NET_WM_STATE client message for sticky & below.\n";
        }

        XFlush(display);
        XCloseDisplay(display);
    }

    else {
        std::cerr << "[platform] Unsupported platform for sticky window.\n";
    }

#elif defined(_WIN32)
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) {
        qWarning() << "[platform] Failed to set HWND_NOTOPMOST.";
    } else {
        qDebug() << "[platform] Set HWND_NOTOPMOST (keeps behind).";
    }

#elif defined(__APPLE__)
    id nswindow = reinterpret_cast<id>(window->winId());
    if (nswindow) {
        constexpr int JoinAllSpaces    = 2;
        constexpr int FullScreenAux    = 128;

        objc_msgSend(nswindow, sel_registerName("setCollectionBehavior:"),
                     JoinAllSpaces | FullScreenAux);

        objc_msgSend(nswindow, sel_registerName("setLevel:"), 0); // NSNormalWindowLevel = 0

        qDebug() << "[platform] Set macOS join all spaces & normal level.";
    } else {
        qWarning() << "[platform] Failed to get macOS NSWindow.";
    }

#else
    qWarning() << "[platform] make_window_sticky unsupported platform.";
#endif
}

} /// namespace platform

#endif /// PLATFORM_WINDOW_STICKY_HPP
