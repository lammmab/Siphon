#include "test_disc_common.h"
#include <stdio.h>

int main(void) {
    DiscTestConfig cfg = {
        .fixture              = SIPHON_TEST_ROOT "/test/fixtures/disc/gc/game.gcz",
        .extract_dir          = SIPHON_TEST_ROOT "/test/out/gcz_gc",
        .expected_format      = "GCZ",
        .expected_game_id     = "TERRIF",
        .expected_entry_count = 1,
    };

    int failures = run_disc_tests(&cfg);
    if (failures == 0)
        printf("all tests passed\n");
    return failures;
}
