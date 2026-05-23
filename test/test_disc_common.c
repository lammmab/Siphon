#include "test_disc_common.h"
#include "siphon.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    char   buf[2048];
    size_t len;
} LogCapture;

static void capture_log(void* userdata, const char* msg) {
    LogCapture* lc = (LogCapture*)userdata;
    size_t room = sizeof(lc->buf) - lc->len - 1;
    if (room == 0) return;
    size_t n = strlen(msg);
    if (n > room) n = room;
    memcpy(lc->buf + lc->len, msg, n);
    lc->len += n;
    lc->buf[lc->len] = '\0';
}

static void silent_log(void* u, const char* m) { (void)u; (void)m; }

static int test_inspect_ok(const DiscTestConfig* cfg) {
    SiphonDiscInfo info;
    SiphonError err = siphon_disc_inspect(cfg->fixture, &info, silent_log, NULL);
    if (err != SIPHON_OK) {
        fprintf(stderr, "inspect returned %d, expected SIPHON_OK\n", err);
        return 1;
    }
    return 0;
}

static int test_inspect_format(const DiscTestConfig* cfg) {
    SiphonDiscInfo info;
    siphon_disc_inspect(cfg->fixture, &info, silent_log, NULL);
    if (strcmp(info.format, cfg->expected_format) != 0) {
        fprintf(stderr, "format: got '%s', expected '%s'\n",
                info.format, cfg->expected_format);
        return 1;
    }
    return 0;
}

static int test_inspect_game_id(const DiscTestConfig* cfg) {
    if (!cfg->expected_game_id) return 0;
    SiphonDiscInfo info;
    siphon_disc_inspect(cfg->fixture, &info, silent_log, NULL);
    if (strncmp(info.game_id, cfg->expected_game_id,
                strlen(cfg->expected_game_id)) != 0) {
        fprintf(stderr, "game_id: got '%.6s', expected '%s'\n",
                info.game_id, cfg->expected_game_id);
        return 1;
    }
    return 0;
}

static int test_inspect_entry_count(const DiscTestConfig* cfg) {
    if (cfg->expected_entry_count < 0) return 0;
    SiphonDiscInfo info;
    siphon_disc_inspect(cfg->fixture, &info, silent_log, NULL);
    if (info.entry_count != cfg->expected_entry_count) {
        fprintf(stderr, "entry_count: got %d, expected %d\n",
                info.entry_count, cfg->expected_entry_count);
        return 1;
    }
    return 0;
}

static int test_extract_ok(const DiscTestConfig* cfg) {
    SiphonError err = siphon_disc_extract(
        cfg->fixture, cfg->extract_dir,
        NULL, 0,
        silent_log, NULL
    );
    if (err != SIPHON_OK) {
        fprintf(stderr, "extract returned %d, expected SIPHON_OK\n", err);
        return 1;
    }
    return 0;
}

static int test_extract_id_match(const DiscTestConfig* cfg) {
    if (!cfg->expected_game_id) return 0;
    const char* ids[] = { cfg->expected_game_id };
    SiphonError err = siphon_disc_extract(
        cfg->fixture, cfg->extract_dir,
        ids, 1,
        silent_log, NULL
    );
    if (err != SIPHON_OK) {
        fprintf(stderr, "extract with matching ID returned %d\n", err);
        return 1;
    }
    return 0;
}

static int test_extract_id_mismatch(const DiscTestConfig* cfg) {
    if (!cfg->expected_game_id) return 0;
    const char* ids[] = { "NOTMATCH" };
    SiphonError err = siphon_disc_extract(
        cfg->fixture, cfg->extract_dir,
        ids, 1,
        silent_log, NULL
    );
    if (err != SIPHON_ERR_ID_MISMATCH) {
        fprintf(stderr, "expected SIPHON_ERR_ID_MISMATCH, got %d\n", err);
        return 1;
    }
    return 0;
}

static int test_inspect_bad_file(const DiscTestConfig* cfg) {
    (void)cfg;
    SiphonDiscInfo info;
    SiphonError err = siphon_disc_inspect(
        SIPHON_TEST_ROOT "/test/fixtures/disc/garbage.bin",
        &info, silent_log, NULL
    );
    if (err == SIPHON_OK) {
        fprintf(stderr, "bad file accepted as SIPHON_OK; should have errored\n");
        return 1;
    }
    return 0;
}

static int test_log_on_error(const DiscTestConfig* cfg) {
    (void)cfg;
    LogCapture lc = {0};
    SiphonDiscInfo info;
    siphon_disc_inspect(
        SIPHON_TEST_ROOT "/test/fixtures/disc/garbage.bin",
        &info, capture_log, &lc
    );
    if (lc.len == 0) {
        fprintf(stderr, "no log output on bad-file inspect\n");
        return 1;
    }
    return 0;
}

#define RUN(fn) do { \
    if ((fn)(cfg) != 0) { failures++; fprintf(stderr, "FAIL " #fn "\n"); } \
} while (0)

int run_disc_tests(const DiscTestConfig* cfg) {
    int failures = 0;

    RUN(test_inspect_ok);
    RUN(test_inspect_format);
    RUN(test_inspect_game_id);
    RUN(test_inspect_entry_count);
    RUN(test_extract_ok);
    RUN(test_extract_id_match);
    RUN(test_extract_id_mismatch);
    RUN(test_inspect_bad_file);
    RUN(test_log_on_error);

    return failures;
}

#undef RUN