#ifndef PLATFORM_WINDOW_STICKY_HPP
#define PLATFORM_WINDOW_STICKY_HPP

#include <QWindow>
#include <QPointer>
#include <QDebug>

#if defined(__linux__)
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <cstring>
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <objc/objc-runtime.h>
#endif

namespace platform {

inline void make_window_sticky(QPointer<QWindow> window = nullptr) {
    if (!window) {
        qWarning() << "[platform] No window provided for make_window_sticky.";
        return;
    }

#if defined(__linux__)
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        qWarning() << "[platform] Failed to open X11 display.";
        return;
    }

    const Window win_id = window->winId();

    /// Helper to intern atom
    auto get_atom = [&](const char* name) -> Atom {
        Atom atom = XInternAtom(display, name, False);
        if (atom == None) {
            qWarning() << "[platform] Failed to get X11 atom:" << name;
        }
        return atom;
    };

    /// _NET_WM_DESKTOP hint (appear on all desktops)
    Atom desktop_atom = get_atom("_NET_WM_DESKTOP");
    if (desktop_atom != None) {
        constexpr unsigned long ALL_DESKTOPS = 0xFFFFFFFF;
        XChangeProperty(display, win_id, desktop_atom, XA_CARDINAL, 32, PropModeReplace,
                        reinterpret_cast<unsigned char*>(const_cast<unsigned long*>(&ALL_DESKTOPS)), 1);
        qDebug() << "[platform] Set _NET_WM_DESKTOP to all desktops.";
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
        e.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
        e.xclient.data.l[1] = sticky;
        e.xclient.data.l[2] = below;

        XSendEvent(display, DefaultRootWindow(display), False,
                   SubstructureRedirectMask | SubstructureNotifyMask, &e);

        qDebug() << "[platform] Sent _NET_WM_STATE client message for sticky & below.";
    }

    XFlush(display);
    XCloseDisplay(display);

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
