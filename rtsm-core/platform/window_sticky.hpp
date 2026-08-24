/// MIT License
#ifndef PLATFORM_WINDOW_STICKY_HPP
#define PLATFORM_WINDOW_STICKY_HPP

#include <iostream>
#include <cstring>

#if __has_include(<QWindow>)
#include <QPointer>
#include <QString>
#include <QWindow>
#include <QDebug>
#endif

#if defined(__linux__)
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#endif

// If GLFW was included before this header, enable GLFW window support
#ifdef GLFW_VERSION_MAJOR
#ifndef GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>
#endif

namespace platform
{

#if __has_include(<QWindow>)
inline void make_window_sticky(QPointer<QWindow> window = nullptr,
			       QString platform_name = QString())
{
	if (!window) {
		std::cerr << "[platform] No window provided for make_window_sticky.\n";
		return;
	}

#if defined(__linux__)
	if (platform_name.startsWith("wayland", Qt::CaseInsensitive)) {
		std::clog << "[Wayland] Currently LayerShell don'nt supported on wayland\n";
		return;
	}

	else if (platform_name.startsWith("xcb", Qt::CaseInsensitive) ||
		 platform_name.contains("x11", Qt::CaseInsensitive)) {
		Display *display = XOpenDisplay(nullptr);
		if (!display) {
			std::cerr << "[platform] Failed to open X11 display.\n";
			return;
		}

		const Window win_id = window->winId();

		/// Helper to intern atom
		auto get_atom = [&](const char *name) -> Atom {
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
			XChangeProperty(display,
					win_id,
					desktop_atom,
					XA_CARDINAL,
					32,
					PropModeReplace,
					reinterpret_cast<unsigned char *>(
						const_cast<unsigned long *>(&ALL_DESKTOPS)),
					1);
			std::clog << "[platform] Set _NET_WM_DESKTOP to all desktops.\n";
		}

		/// Proper client message for _NET_WM_STATE sticky + below + skip taskbar + skip pager
		Atom wm_state = get_atom("_NET_WM_STATE");
		Atom sticky = get_atom("_NET_WM_STATE_STICKY");
		Atom below = get_atom("_NET_WM_STATE_BELOW");
		Atom skip_taskbar = get_atom("_NET_WM_STATE_SKIP_TASKBAR");
		Atom skip_pager = get_atom("_NET_WM_STATE_SKIP_PAGER");

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

			XSendEvent(display,
				   DefaultRootWindow(display),
				   False,
				   SubstructureRedirectMask | SubstructureNotifyMask,
				   &e);

			std::clog
				<< "[platform] Sent _NET_WM_STATE client message for sticky & below.\n";
		}

		if (wm_state && skip_taskbar && skip_pager) {
			XEvent e;
			std::memset(&e, 0, sizeof(e));
			e.xclient.type = ClientMessage;
			e.xclient.message_type = wm_state;
			e.xclient.display = display;
			e.xclient.window = win_id;
			e.xclient.format = 32;
			e.xclient.data.l[0] = 1; /// _NET_WM_STATE_ADD
			e.xclient.data.l[1] = skip_taskbar;
			e.xclient.data.l[2] = skip_pager;

			XSendEvent(display,
				   DefaultRootWindow(display),
				   False,
				   SubstructureRedirectMask | SubstructureNotifyMask,
				   &e);

			std::clog
				<< "[platform] Sent _NET_WM_STATE client message for skip taskbar & pager.\n";
		}

		XFlush(display);
		XCloseDisplay(display);
	}
	else {
		std::cerr << "[platform] Unsupported platform for sticky window.\n";
	}

#elif defined(_WIN32)
	// HWND hwnd = reinterpret_cast<HWND>(window->winId());
#elif defined(__APPLE__)
	// id nswindow = reinterpret_cast<id>(window->winId());
#else
	qWarning() << "[platform] make_window_sticky unsupported platform.";
#endif
}
#endif

#ifdef GLFW_VERSION_MAJOR
inline void make_window_sticky(GLFWwindow* window)
{
    if (!window) {
        std::cerr << "[platform] No window provided for make_window_sticky.\n";
        return;
    }

#if defined(__linux__)
    int platform_type = glfwGetPlatform();
    if (platform_type == GLFW_PLATFORM_WAYLAND) {
        std::clog << "[Wayland] Currently LayerShell don'nt supported on wayland\n";
        return;
    }
    
    if (platform_type == GLFW_PLATFORM_X11) {
        Display* display = glfwGetX11Display();
        Window win_id = glfwGetX11Window(window);

        if (display && win_id) {
            auto get_atom = [&](const char* name) -> Atom {
                Atom atom = XInternAtom(display, name, False);
                if (atom == None) {
                    std::clog << "[platform] Failed to get X11 atom: " << name << "\n";
                }
                return atom;
            };

            Atom desktop_atom = get_atom("_NET_WM_DESKTOP");
            if (desktop_atom != None) {
                constexpr unsigned long ALL_DESKTOPS = 0xFFFFFFFF;
                XChangeProperty(display, win_id, desktop_atom, XA_CARDINAL, 32,
                                PropModeReplace, reinterpret_cast<const unsigned char*>(&ALL_DESKTOPS), 1);
                std::clog << "[platform] Set _NET_WM_DESKTOP to all desktops.\n";
            }

            Atom wm_state = get_atom("_NET_WM_STATE");
            Atom sticky = get_atom("_NET_WM_STATE_STICKY");
            Atom below = get_atom("_NET_WM_STATE_BELOW");
            Atom skip_taskbar = get_atom("_NET_WM_STATE_SKIP_TASKBAR");
            Atom skip_pager = get_atom("_NET_WM_STATE_SKIP_PAGER");

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
                std::clog << "[platform] Sent _NET_WM_STATE client message for sticky & below.\n";
            }

            if (wm_state && skip_taskbar && skip_pager) {
                XEvent e;
                std::memset(&e, 0, sizeof(e));
                e.xclient.type = ClientMessage;
                e.xclient.message_type = wm_state;
                e.xclient.display = display;
                e.xclient.window = win_id;
                e.xclient.format = 32;
                e.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
                e.xclient.data.l[1] = skip_taskbar;
                e.xclient.data.l[2] = skip_pager;

                XSendEvent(display, DefaultRootWindow(display), False,
                           SubstructureRedirectMask | SubstructureNotifyMask, &e);
                std::clog << "[platform] Sent _NET_WM_STATE client message for skip taskbar & pager.\n";
            }

            // Force hide title bar via _MOTIF_WM_HINTS (in case GLFW_DECORATED fails on XWayland)
            Atom motif_hints = get_atom("_MOTIF_WM_HINTS");
            if (motif_hints != None) {
                struct {
                    unsigned long flags;
                    unsigned long functions;
                    unsigned long decorations;
                    long input_mode;
                    unsigned long status;
                } hints = {2, 0, 0, 0, 0}; // MWM_HINTS_DECORATIONS = 2, decorations = 0
                XChangeProperty(display, win_id, motif_hints, motif_hints, 32,
                                PropModeReplace, reinterpret_cast<const unsigned char*>(&hints), 5);
                std::clog << "[platform] Set _MOTIF_WM_HINTS to disable decorations.\n";
            }

            XFlush(display);
        }
    }
#endif
}
#endif

} /// namespace platform

#endif /// PLATFORM_WINDOW_STICKY_HPP
