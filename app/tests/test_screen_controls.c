#include "common.h"

#include <assert.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "screen_controls.h"

static bool
overlap(SDL_Rect a, SDL_Rect b) {
    return a.x < b.x + b.w && b.x < a.x + a.w
        && a.y < b.y + b.h && b.y < a.y + a.h;
}

static void
verify(int width, int height, bool ledger) {
    struct sc_shell_layout l;
    sc_shell_compute(width, height, ledger, &l);
    assert(l.viewport.w > 0 && l.viewport.h > 0);
    assert(l.viewport.x >= 0 && l.viewport.y >= 0);
    assert(l.viewport.x + l.viewport.w <= width);
    assert(l.viewport.y + l.viewport.h <= height);
    assert(sc_shell_is_viewport(&l, l.viewport.x + l.viewport.w / 2,
                               l.viewport.y + l.viewport.h / 2));
    for (int i = 0; i < SC_SHELL_BUTTON_COUNT; ++i) {
        SDL_Rect b = l.buttons[i];
        assert(b.w > 0 && b.h > 0);
        assert(b.x >= 0 && b.y >= 0);
        assert(b.x + b.w <= width && b.y + b.h <= height);
        assert(!overlap(l.viewport, b));
        assert(sc_shell_hit_test(&l, b.x + b.w / 2, b.y + b.h / 2) == i);
        assert(strlen(sc_shell_button_label((enum sc_shell_button) i)) > 0);
        for (int j = i + 1; j < SC_SHELL_BUTTON_COUNT; ++j) {
            assert(!overlap(b, l.buttons[j]));
        }
    }
    assert(sc_shell_hit_test(&l, l.ledger_toggle.x + 3,
                             l.ledger_toggle.y + 3) == SC_SHELL_LEDGER);
    assert(sc_shell_hit_test(&l, l.fullscreen_toggle.x + 3,
                             l.fullscreen_toggle.y + 3) == SC_SHELL_FULLSCREEN);
}

int
main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    verify(360, 720, false);
    verify(360, 720, true);
    verify(600, 900, false);
    verify(800, 500, false);
    verify(1280, 900, true);
    return 0;
}
