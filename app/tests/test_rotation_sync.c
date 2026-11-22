#include "common.h"
#include <assert.h>
#include "rotation_sync.h"

static void test_cycle(void) {
    enum sc_rotation_mode mode = SC_ROTATION_MODE_AUTO;
    for (unsigned i = 0; i < SC_ROTATION_MODE_COUNT; ++i) {
        assert(mode == (enum sc_rotation_mode) i);
        mode = sc_rotation_sync_next_mode(mode);
    }
    assert(mode == SC_ROTATION_MODE_AUTO);
}

static void test_mapping(void) {
    static const uint8_t expected[] = {0, 0, 1, 2, 3};
    uint8_t rotation = 255;
    assert(!sc_rotation_sync_mode_to_rotation(SC_ROTATION_MODE_AUTO, &rotation));
    for (unsigned i = 1; i < SC_ROTATION_MODE_COUNT; ++i) {
        assert(sc_rotation_sync_mode_to_rotation((enum sc_rotation_mode) i, &rotation));
        assert(rotation == expected[i]);
    }
}

static void test_auto_tracks_device(void) {
    struct sc_rotation_sync_state state;
    sc_rotation_sync_init(&state);
    assert(sc_rotation_sync_update_device(&state, 1));
    uint8_t rotation = 0;
    assert(sc_rotation_sync_get_sdl_rotation(&state, &rotation) && rotation == 1);
    assert(sc_rotation_sync_update_device(&state, 3));
    assert(sc_rotation_sync_get_sdl_rotation(&state, &rotation) && rotation == 3);
}

static void test_forced_mode_waits_for_ack(void) {
    struct sc_rotation_sync_state state;
    sc_rotation_sync_init(&state);
    assert(sc_rotation_sync_update_device(&state, 0));
    assert(!sc_rotation_sync_set_requested_mode(&state, SC_ROTATION_MODE_REVERSE_LANDSCAPE));
    assert(state.status == SC_ROTATION_SYNC_PENDING);
    uint8_t rotation = 99;
    assert(!sc_rotation_sync_get_sdl_rotation(&state, &rotation));
    assert(sc_rotation_sync_update_device(&state, 3));
    assert(sc_rotation_sync_get_sdl_rotation(&state, &rotation) && rotation == 3);
}

static void test_external_change_in_auto(void) {
    struct sc_rotation_sync_state state;
    sc_rotation_sync_init(&state);
    assert(sc_rotation_sync_update_device(&state, 0));
    assert(sc_rotation_sync_update_device(&state, 2));
    assert(state.current_sdl_rotation == 2 && state.status == SC_ROTATION_SYNCED);
}

int main(int argc, char *argv[]) {
    (void) argc; (void) argv;
    test_cycle(); test_mapping(); test_auto_tracks_device();
    test_forced_mode_waits_for_ack(); test_external_change_in_auto();
    return 0;
}
