// Seat.hpp - Keyboard (xkbcommon) and pointer input for the lock surfaces.
//
// Translates raw Wayland input into text / special keys / pointer events and
// forwards them to an InputSink. Owns key-repeat via the event loop's timers.

#pragma once

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include <cstdint>
#include <functional>

namespace qypr {

class EventLoop;
class InputSink;
class Output;

// Modifier bitmask passed with special keys.
enum Mod : uint32_t { MOD_SHIFT = 1u << 0, MOD_CTRL = 1u << 1, MOD_ALT = 1u << 2 };

class Seat {
public:
    Seat(wl_seat* seat, EventLoop& loop);
    ~Seat();

    void setSink(InputSink* sink) { sink_ = sink; }
    // Resolve a focused wl_surface to the Output that owns it (for size).
    void setOutputResolver(std::function<Output*(wl_surface*)> fn) {
        outputForSurface_ = std::move(fn);
    }

    // Wayland C callbacks (public so listener tables at file scope can bind them).
    // wl_seat
    static void onCapabilities(void*, wl_seat*, uint32_t);
    static void onSeatName(void*, wl_seat*, const char*);

    // wl_keyboard
    static void onKeymap(void*, wl_keyboard*, uint32_t, int32_t, uint32_t);
    static void onKbEnter(void*, wl_keyboard*, uint32_t, wl_surface*, wl_array*);
    static void onKbLeave(void*, wl_keyboard*, uint32_t, wl_surface*);
    static void onKey(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t);
    static void onModifiers(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    static void onRepeatInfo(void*, wl_keyboard*, int32_t, int32_t);

    // wl_pointer
    static void onPtrEnter(void*, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t);
    static void onPtrLeave(void*, wl_pointer*, uint32_t, wl_surface*);
    static void onPtrMotion(void*, wl_pointer*, uint32_t, wl_fixed_t, wl_fixed_t);
    static void onPtrButton(void*, wl_pointer*, uint32_t, uint32_t, uint32_t, uint32_t);
    static void onPtrAxis(void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t);

private:
    void handleKey(uint32_t keycode);
    void startRepeat(uint32_t keycode);
    void stopRepeat();

    wl_seat* seat_ = nullptr;
    EventLoop& loop_;
    InputSink* sink_ = nullptr;
    std::function<Output*(wl_surface*)> outputForSurface_;

    wl_keyboard* keyboard_ = nullptr;
    wl_pointer* pointer_ = nullptr;

    xkb_context* xkbContext_ = nullptr;
    xkb_keymap* xkbKeymap_ = nullptr;
    xkb_state* xkbState_ = nullptr;

    // Pointer focus
    Output* pointerOutput_ = nullptr;
    double ptrX_ = 0, ptrY_ = 0;

    // Key repeat
    int repeatRate_ = 25;    // keys per second
    int repeatDelay_ = 600;  // ms before repeat begins
    uint32_t repeatKeycode_ = 0;
    int repeatTimer_ = -1;
};

}  // namespace qypr
