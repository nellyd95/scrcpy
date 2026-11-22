#include "screen_controls.h"

#include <assert.h>
#include <string.h>

static int
mini(int a, int b) { return a < b ? a : b; }

static int
maxi(int a, int b) { return a > b ? a : b; }

static bool
contains(const SDL_Rect *r, int x, int y) {
    return r->w > 0 && r->h > 0 && x >= r->x && y >= r->y
        && x < r->x + r->w && y < r->y + r->h;
}

void
sc_shell_compute(int32_t width, int32_t height, bool ledger,
                 struct sc_shell_layout *l) {
    assert(l && width > 0 && height > 0);
    int side = mini(SC_SHELL_SIDE, maxi(0, width / 5));
    int top = mini(SC_SHELL_TOP, maxi(0, height / 6));
    int bottom = mini(SC_SHELL_BOTTOM
                      + (ledger ? SC_SHELL_LEDGER_EXTRA : 0),
                      maxi(0, height / 3));
    l->header = (SDL_Rect) {0, 0, width, top};
    l->footer = (SDL_Rect) {0, height - bottom, width, bottom};
    l->viewport = (SDL_Rect) {
        side, top, maxi(1, width - 2 * side),
        maxi(1, height - top - bottom),
    };
    int rail_top = top + 22;
    int rail_height = maxi(24, mini(42, (height - top - bottom - 36) / 4));
    int rail_gap = maxi(4, rail_height / 3);
    int right = width - side + 3;
    int bw = maxi(10, side - 8);
    l->buttons[SC_SHELL_POWER] = (SDL_Rect) {3, rail_top, bw, rail_height};
    l->buttons[SC_SHELL_NOTIFICATIONS] =
        (SDL_Rect) {3, rail_top + rail_height + rail_gap, bw, rail_height};
    l->buttons[SC_SHELL_BACK] =
        (SDL_Rect) {3, rail_top + 2 * (rail_height + rail_gap), bw, rail_height};
    l->buttons[SC_SHELL_VOLUME_UP] =
        (SDL_Rect) {right, rail_top, bw, rail_height};
    l->buttons[SC_SHELL_VOLUME_DOWN] =
        (SDL_Rect) {right, rail_top + rail_height + rail_gap, bw, rail_height};
    l->buttons[SC_SHELL_MUTE] =
        (SDL_Rect) {right, rail_top + 2 * (rail_height + rail_gap), bw, rail_height};

    int pad = mini(12, width / 30);
    int gap = mini(7, width / 50);
    int available = width - 2 * pad - 4 * gap;
    int button_width = maxi(1, available / 5);
    int row_height = mini(45, maxi(18, bottom / (ledger ? 4 : 2)));
    int y = height - row_height - 13;
    enum sc_shell_button nav[4] = {
        SC_SHELL_HOME, SC_SHELL_APPS, SC_SHELL_SETTINGS, SC_SHELL_ROTATION
    };
    for (unsigned i = 0; i < 4; ++i) {
        int x = pad + (int)i * (button_width + gap);
        l->buttons[nav[i]] = (SDL_Rect) {x, y, button_width, row_height};
    }
    l->fullscreen_toggle = (SDL_Rect) {maxi(0, width - 67), 9, 58, maxi(10, top - 17)};
    l->ledger_toggle = (SDL_Rect) {maxi(0, width - 140), 9, 67, maxi(10, top - 17)};
}

enum sc_shell_button
sc_shell_hit_test(const struct sc_shell_layout *l, int32_t x, int32_t y) {
    for (int i = 0; i < SC_SHELL_BUTTON_COUNT; ++i) {
        if (contains(&l->buttons[i], x, y)) {
            return (enum sc_shell_button) i;
        }
    }
    if (contains(&l->ledger_toggle, x, y)) return SC_SHELL_LEDGER;
    if (contains(&l->fullscreen_toggle, x, y)) return SC_SHELL_FULLSCREEN;
    return SC_SHELL_NONE;
}

bool
sc_shell_is_viewport(const struct sc_shell_layout *l, int32_t x, int32_t y) {
    return contains(&l->viewport, x, y);
}

const char *
sc_shell_button_label(enum sc_shell_button button) {
    static const char *const labels[SC_SHELL_BUTTON_COUNT] = {
        "PWR", "VOL+", "VOL-", "MUTE", "BACK", "HOME", "APPS",
        "NOTIF", "SET", "ROTATE"
    };
    return button >= 0 && button < SC_SHELL_BUTTON_COUNT
         ? labels[button] : "";
}

static void
fill(SDL_Renderer *r, SDL_FRect rect, uint8_t red, uint8_t green,
     uint8_t blue) {
    SDL_SetRenderDrawColor(r, red, green, blue, 255);
    SDL_RenderFillRect(r, &rect);
}

static SDL_FRect
scaled(SDL_Rect rect, float s) {
    return (SDL_FRect) {rect.x * s, rect.y * s, rect.w * s, rect.h * s};
}

static void
draw_text(SDL_Renderer *r, float x, float y, const char *text) {
    SDL_SetRenderDrawColor(r, 218, 230, 245, 255);
    SDL_RenderDebugText(r, x, y, text);
}

static void
button(SDL_Renderer *r, SDL_Rect rect, float scale, const char *label,
       bool selected, bool pressed, bool enabled) {
    SDL_FRect area = scaled(rect, scale);
    if (!enabled) {
        fill(r, area, 44, 45, 52);
    } else if (pressed) {
        fill(r, area, 26, 114, 162);
    } else if (selected) {
        fill(r, area, 46, 93, 132);
    } else {
        fill(r, area, 36, 52, 73);
    }
    SDL_SetRenderDrawColor(r, enabled ? 64 : 62, enabled ? 156 : 62,
                           enabled ? 190 : 62, 255);
    SDL_RenderRect(r, &area);
    float text_width = (float)strlen(label) * 8;
    draw_text(r, area.x + maxi(2, ((int)area.w - (int)text_width) / 2),
              area.y + maxi(2, ((int)area.h - 8) / 2), label);
}

void
sc_shell_draw(SDL_Renderer *r, const struct sc_shell_layout *l,
              float scale, enum sc_shell_button hover,
              enum sc_shell_button pressed, bool enabled, bool ledger,
              const char *rotation_mode, const char *sync_status) {
    SDL_FRect header = scaled(l->header, scale);
    SDL_FRect footer = scaled(l->footer, scale);
    fill(r, header, 15, 24, 38);
    fill(r, footer, 15, 24, 38);
    SDL_Rect left = {0, l->viewport.y, l->viewport.x, l->viewport.h};
    SDL_Rect right = {l->viewport.x + l->viewport.w, l->viewport.y,
                      l->viewport.x, l->viewport.h};
    fill(r, scaled(left, scale), 20, 29, 43);
    fill(r, scaled(right, scale), 20, 29, 43);

    SDL_FRect edge = scaled(l->viewport, scale);
    SDL_SetRenderDrawColor(r, 66, 91, 113, 255);
    SDL_RenderRect(r, &edge);

    draw_text(r, 12 * scale, 10 * scale, "SCRCPY  /  PHONE SHELL");
    draw_text(r, 12 * scale, 27 * scale, enabled ? "USB CONTROL ACTIVE" : "CONTROL DISABLED");
    button(r, l->ledger_toggle, scale, "LEDGER", hover == SC_SHELL_LEDGER,
           pressed == SC_SHELL_LEDGER, true);
    button(r, l->fullscreen_toggle, scale, "FULL", hover == SC_SHELL_FULLSCREEN,
           pressed == SC_SHELL_FULLSCREEN, true);

    for (int i = 0; i < SC_SHELL_BUTTON_COUNT; ++i) {
        button(r, l->buttons[i], scale,
               sc_shell_button_label((enum sc_shell_button)i),
               hover == i, pressed == i, enabled);
    }

    char status[160];
    SDL_snprintf(status, sizeof(status), "ROTATION: %s   SYNC: %s",
                 rotation_mode ? rotation_mode : "AUTO",
                 sync_status ? sync_status : "UNKNOWN");
    draw_text(r, 12 * scale, (l->footer.y + 7) * scale, status);

    if (ledger) {
        int y = l->footer.y + 26;
        draw_text(r, 12 * scale, y * scale,
                  "PHYSICAL BUTTONS: PWR / VOL+ / VOL- / MUTE");
        draw_text(r, 12 * scale, (y + 15) * scale,
                  "DEVICE: BACK / HOME / APPS / NOTIF / SET / ROTATE");
        draw_text(r, 12 * scale, (y + 30) * scale,
                  "HIDDEN / INACTIVE: CAMERA ZOOM, TORCH, GAMEPAD");
        draw_text(r, 12 * scale, (y + 45) * scale,
                  "HIDDEN / INACTIVE: SCREEN RECORD (CLI ONLY)");
    }
}
