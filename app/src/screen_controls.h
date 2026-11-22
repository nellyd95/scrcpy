#ifndef SC_SCREEN_CONTROLS_H
#define SC_SCREEN_CONTROLS_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#define SC_SHELL_TOP 46
#define SC_SHELL_SIDE 42
#define SC_SHELL_BOTTOM 84
#define SC_SHELL_LEDGER_EXTRA 105

enum sc_shell_button {
    SC_SHELL_NONE = -1,
    SC_SHELL_POWER,
    SC_SHELL_VOLUME_UP,
    SC_SHELL_VOLUME_DOWN,
    SC_SHELL_MUTE,
    SC_SHELL_BACK,
    SC_SHELL_HOME,
    SC_SHELL_APPS,
    SC_SHELL_NOTIFICATIONS,
    SC_SHELL_SETTINGS,
    SC_SHELL_ROTATION,
    SC_SHELL_BUTTON_COUNT,
    SC_SHELL_LEDGER = SC_SHELL_BUTTON_COUNT,
    SC_SHELL_FULLSCREEN,
};

struct sc_shell_layout {
    SDL_Rect viewport;
    SDL_Rect buttons[SC_SHELL_BUTTON_COUNT];
    SDL_Rect ledger_toggle;
    SDL_Rect fullscreen_toggle;
    SDL_Rect footer;
    SDL_Rect header;
};

void
sc_shell_compute(int32_t width, int32_t height, bool ledger,
                 struct sc_shell_layout *layout);

enum sc_shell_button
sc_shell_hit_test(const struct sc_shell_layout *layout, int32_t x, int32_t y);

bool
sc_shell_is_viewport(const struct sc_shell_layout *layout, int32_t x, int32_t y);

const char *
sc_shell_button_label(enum sc_shell_button button);

void
sc_shell_draw(SDL_Renderer *renderer, const struct sc_shell_layout *layout,
              float scale, enum sc_shell_button hover,
              enum sc_shell_button pressed, bool enabled, bool ledger,
              const char *rotation_mode, const char *sync_status);

#endif
