#ifndef SC_ROTATION_SYNC_H
#define SC_ROTATION_SYNC_H

#include "common.h"
#include <stdbool.h>
#include <stdint.h>

enum sc_rotation_mode {
    SC_ROTATION_MODE_AUTO,
    SC_ROTATION_MODE_PORTRAIT,
    SC_ROTATION_MODE_LANDSCAPE,
    SC_ROTATION_MODE_REVERSE_PORTRAIT,
    SC_ROTATION_MODE_REVERSE_LANDSCAPE,
    SC_ROTATION_MODE_COUNT,
};

enum sc_rotation_sync_status {
    SC_ROTATION_SYNC_UNKNOWN,
    SC_ROTATION_SYNC_PENDING,
    SC_ROTATION_SYNCED,
};

struct sc_rotation_sync_state {
    enum sc_rotation_mode requested_mode;
    uint8_t confirmed_device_rotation;
    uint8_t current_sdl_rotation;
    enum sc_rotation_sync_status status;
    bool has_device_rotation;
    bool device_rotation_frozen;
    // A local UI request is outstanding until Android reports a match.
    // Otherwise the device is authoritative, including external changes.
    bool request_pending;
};

void sc_rotation_sync_init(struct sc_rotation_sync_state *state);
enum sc_rotation_mode sc_rotation_sync_next_mode(enum sc_rotation_mode current);
bool sc_rotation_sync_mode_to_rotation(enum sc_rotation_mode mode, uint8_t *rotation);
bool sc_rotation_sync_update_device(struct sc_rotation_sync_state *state, uint8_t device_rotation);
bool sc_rotation_sync_update_device_state(struct sc_rotation_sync_state *state,
                                          uint8_t device_rotation, bool frozen);
bool sc_rotation_sync_set_requested_mode(struct sc_rotation_sync_state *state, enum sc_rotation_mode mode);
bool sc_rotation_sync_get_sdl_rotation(const struct sc_rotation_sync_state *state, uint8_t *rotation);
const char *sc_rotation_mode_get_name(enum sc_rotation_mode mode);

#endif
