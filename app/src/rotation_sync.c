#include "rotation_sync.h"
#include <assert.h>

void sc_rotation_sync_init(struct sc_rotation_sync_state *state) {
    state->requested_mode = SC_ROTATION_MODE_AUTO;
    state->confirmed_device_rotation = 0;
    state->current_sdl_rotation = 0;
    state->status = SC_ROTATION_SYNC_UNKNOWN;
    state->has_device_rotation = false;
    state->device_rotation_frozen = false;
}

enum sc_rotation_mode sc_rotation_sync_next_mode(enum sc_rotation_mode current) {
    assert(current >= SC_ROTATION_MODE_AUTO && current < SC_ROTATION_MODE_COUNT);
    return (enum sc_rotation_mode) ((current + 1) % SC_ROTATION_MODE_COUNT);
}

bool sc_rotation_sync_mode_to_rotation(enum sc_rotation_mode mode, uint8_t *rotation) {
    assert(rotation);
    switch (mode) {
        case SC_ROTATION_MODE_AUTO: return false;
        case SC_ROTATION_MODE_PORTRAIT: *rotation = 0; return true;
        case SC_ROTATION_MODE_LANDSCAPE: *rotation = 1; return true;
        case SC_ROTATION_MODE_REVERSE_PORTRAIT: *rotation = 2; return true;
        case SC_ROTATION_MODE_REVERSE_LANDSCAPE: *rotation = 3; return true;
        case SC_ROTATION_MODE_COUNT: return false;
    }
    return false;
}

bool sc_rotation_sync_update_device_state(struct sc_rotation_sync_state *state,
                                          uint8_t device_rotation, bool frozen) {
    assert(device_rotation < 4);
    state->confirmed_device_rotation = device_rotation;
    state->current_sdl_rotation = device_rotation;
    state->has_device_rotation = true;
    state->device_rotation_frozen = frozen;

    uint8_t requested_rotation = 0;
    bool forced = sc_rotation_sync_mode_to_rotation(state->requested_mode,
                                                     &requested_rotation);
    bool matched = forced ? (frozen && requested_rotation == device_rotation)
                          : !frozen;
    state->status = matched ? SC_ROTATION_SYNCED : SC_ROTATION_SYNC_PENDING;
    return matched;
}

bool sc_rotation_sync_update_device(struct sc_rotation_sync_state *state,
                                   uint8_t device_rotation) {
    // Convenience for the existing pure-rotation tests. The protocol uses
    // sc_rotation_sync_update_device_state() with Android's actual freeze flag.
    bool inferred_frozen = state->requested_mode != SC_ROTATION_MODE_AUTO;
    return sc_rotation_sync_update_device_state(state, device_rotation,
                                                inferred_frozen);
}

bool sc_rotation_sync_set_requested_mode(struct sc_rotation_sync_state *state,
                                        enum sc_rotation_mode mode) {
    assert(mode >= SC_ROTATION_MODE_AUTO && mode < SC_ROTATION_MODE_COUNT);
    state->requested_mode = mode;
    if (!state->has_device_rotation) {
        state->status = SC_ROTATION_SYNC_UNKNOWN;
        return false;
    }

    uint8_t desired = 0;
    bool forced = sc_rotation_sync_mode_to_rotation(mode, &desired);
    bool matched = forced
                 ? (state->device_rotation_frozen
                    && state->confirmed_device_rotation == desired)
                 : !state->device_rotation_frozen;
    state->status = matched ? SC_ROTATION_SYNCED : SC_ROTATION_SYNC_PENDING;
    return matched;
}

bool sc_rotation_sync_get_sdl_rotation(const struct sc_rotation_sync_state *state,
                                       uint8_t *rotation) {
    assert(rotation);
    if (!state->has_device_rotation || state->status != SC_ROTATION_SYNCED) {
        return false;
    }
    *rotation = state->current_sdl_rotation;
    return true;
}

const char *sc_rotation_mode_get_name(enum sc_rotation_mode mode) {
    static const char *const names[] = {
        "Auto (Sensor)", "Portrait (0)", "Landscape (90)",
        "Reverse Portrait (180)", "Reverse Landscape (270)",
    };
    assert(mode >= SC_ROTATION_MODE_AUTO && mode < SC_ROTATION_MODE_COUNT);
    return names[mode];
}
