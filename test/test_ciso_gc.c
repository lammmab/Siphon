#include "test_disc_common.h"
#include <stdio.h>

int main(void) {
    DiscTestConfig cfg = {
        .fixture              = SIPHON_TEST_ROOT "/test/fixtures/mock/gc/game.ciso",
        .extract_dir          = SIPHON_TEST_ROOT "/test/out/ciso",
        .expected_format      = "CISO",
        .expected_game_id     = "SIPHON",
        .expected_entry_count = 1,
    };

    int failures = run_disc_tests(&cfg);
    if (failures == 0)
        printf("all tests passed\n");
    return failures;
}