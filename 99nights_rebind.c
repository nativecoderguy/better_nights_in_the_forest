#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>

// --- Configuration ---
#define KEY_E 14
#define KEY_F 3
#define CAPS_LOCK_CODE 0x39
#define DOUBLE_TAP_THRESHOLD_NS 250000000 // 250ms in nanoseconds

// ANSI KeyCodes for 0-9
const CGKeyCode keycodes[] = { 29, 18, 19, 20, 21, 23, 22, 26, 28, 25 };

// Mouse Button Numbers
#define MOUSE_BTN_BACK 3
#define MOUSE_BTN_FWD  4

// --- State ---
int current_cycle = 2; // Default start
int max_cycle = 9;     // Default range 1-9

// Double Tap State
uint64_t last_key_time = 0;
CGKeyCode last_key_code = 0;

// --- Helpers ---

void send_key(CGKeyCode key) {
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    CGEventRef keyDown = CGEventCreateKeyboardEvent(source, key, true);
    CGEventRef keyUp   = CGEventCreateKeyboardEvent(source, key, false);
    CGEventPost(kCGHIDEventTap, keyDown);
    CGEventPost(kCGHIDEventTap, keyUp);
    CFRelease(keyDown);
    CFRelease(keyUp);
    CFRelease(source);
}

int get_number_from_keycode(CGKeyCode code) {
    for (int i = 1; i <= 9; i++) {
        if (keycodes[i] == code) return i;
    }
    return 0;
}

bool is_caps_lock_on() {
    return CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, CAPS_LOCK_CODE);
}

// --- The Hook ---
CGEventRef eventTapFunction(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {

    // 1. Handle Keyboard (Update State & Double Tap)
    if (type == kCGEventKeyDown) {
        CGKeyCode code = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        int num = get_number_from_keycode(code);

        if (num > 0) {
            uint64_t now = CGEventGetTimestamp(event);

            // Double Tap Detection
            if (code == last_key_code && (now - last_key_time) < DOUBLE_TAP_THRESHOLD_NS) {
                max_cycle = num;
                printf("Max cycle range set to: 1-%d\n", max_cycle);
            }

            last_key_time = now;
            last_key_code = code;

            // Update current state
            current_cycle = num;
        }
        return event; // Always pass through
    }

    // 2. Handle Scroll Wheel (Cycle 1-Max)
    if (type == kCGEventScrollWheel) {
        // If Caps Lock is ON, disable feature (pass event through)
        if (is_caps_lock_on()) {
            return event;
        }

        int64_t delta = CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1);

        if (delta != 0) {
            // Scroll Up (+) -> Next, Scroll Down (-) -> Prev
            if (delta > 0) {
                current_cycle++;
                if (current_cycle > max_cycle) current_cycle = 1;
            } else {
                current_cycle--;
                if (current_cycle < 1) current_cycle = max_cycle;
            }

            send_key(keycodes[current_cycle]);
            return NULL; // Consume scroll event
        }
    }

    // 3. Handle Side Buttons
    if (type == kCGEventOtherMouseDown) {
        int64_t btn = CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber);

        // Side 1 (Back) -> 'e'
        if (btn == MOUSE_BTN_BACK) {
            send_key(KEY_E);
            return NULL;
        }

        // Side 2 (Fwd) -> Smart Macro
        if (btn == MOUSE_BTN_FWD) {
            if (current_cycle == 1) {
                send_key(KEY_F);
            } else {
                // Press 1 -> Press F -> Press Old Number
                send_key(keycodes[1]);
                send_key(KEY_F);
                send_key(keycodes[current_cycle]);
            }
            return NULL;
        }
    }

    return event;
}

int main(void) {
    printf("Starting Mouse Mapper...\n");
    printf("1. Scroll Wheel  : Cycles 1-%d (blocked by Caps Lock)\n", max_cycle);
    printf("2. Side 1 (Back) : 'e'\n");
    printf("3. Side 2 (Fwd)  : 'f' (Smart Return)\n");
    printf("4. Double Tap #  : Sets max cycle range\n");
    printf("   Default Group : %d\n", current_cycle);

    CGEventMask eventMask = CGEventMaskBit(kCGEventOtherMouseDown) |
                            CGEventMaskBit(kCGEventKeyDown) |
                            CGEventMaskBit(kCGEventScrollWheel);

    CFMachPortRef eventTap = CGEventTapCreate(
        kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
        eventMask, eventTapFunction, NULL
    );

    if (!eventTap) {
        fprintf(stderr, "failed to create event tap (sudo required).\n");
        return 1;
    }

    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    CFRunLoopRun();
    return 0;
}
