#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE

#include "input.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

static struct termios g_orig_termios;
static int g_orig_flags;
static int g_active = 0;

// Runs on SIGINT/SIGTERM. Just exits normally so the atexit-registered
// cleanup in renderer.c (which calls input_restore()) still runs.
static void handle_signal(int sig) {
    (void)sig;
    exit(0);
}

void input_init(void) {
    tcgetattr(STDIN_FILENO, &g_orig_termios);
    struct termios raw = g_orig_termios;
    raw.c_lflag &= (unsigned)~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    g_orig_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, g_orig_flags | O_NONBLOCK);

    g_active = 1;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
}

void input_restore(void) {
    if (!g_active) return;
    tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
    fcntl(STDIN_FILENO, F_SETFL, g_orig_flags);
    g_active = 0;
}

InputState input_poll(void) {
    InputState in;
    memset(&in, 0, sizeof(in));

    char buf[64];
    ssize_t n;
    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            switch (buf[i]) {
                case 'w': in.w = true; break;
                case 'a': in.a = true; break;
                case 's': in.s = true; break;
                case 'd': in.d = true; break;
                case 'r': in.r = true; break;
                case 'f': in.f = true; break;
                case 'i': in.i = true; break;
                case 'j': in.j = true; break;
                case 'k': in.k = true; break;
                case 'l': in.l = true; break;
                case 'q': in.quit = true; break;
                default: break;
            }
        }
    }
    return in;
}
