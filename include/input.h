#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

// Which movement/look keys were seen since the last input_poll() call.
// Terminals don't give us real key-up events, so "true" here means
// "this key's byte arrived in the input stream this frame" -- holding
// a key down relies on the OS's keyboard auto-repeat to keep sending it.
typedef struct {
    bool w, a, s, d; // move forward / left / back / right
    bool r, f;       // move up / down (world-vertical, not camera-tilt)
    bool i, k;       // look up / down (pitch)
    bool j, l;       // look left / right (yaw)
    bool quit;       // 'q' pressed
} InputState;

// Puts the terminal into raw, non-blocking mode: no line buffering (keys
// are readable immediately, not after Enter) and no echo (typed keys
// don't spam the screen). Also installs SIGINT/SIGTERM handlers so
// Ctrl+C still restores the terminal instead of leaving it broken.
// Call once at startup.
void input_init(void);

// Restores the terminal to whatever mode it was in before input_init().
// Safe to call multiple times; call this before your program exits.
void input_restore(void);

// Drains all pending key presses (never blocks) and reports which
// movement/look/quit keys were seen.
InputState input_poll(void);

#endif
