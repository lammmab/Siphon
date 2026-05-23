#ifndef TEST_DISC_COMMON_H
#define TEST_DISC_COMMON_H

typedef struct {
    const char* fixture;
    const char* extract_dir;
    const char* expected_format;
    const char* expected_game_id;     // NULL to skip
    int         expected_entry_count; // -1 to skip
} DiscTestConfig;

int run_disc_tests(const DiscTestConfig* cfg);

#endif