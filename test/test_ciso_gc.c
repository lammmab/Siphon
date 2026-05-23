#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "siphon.h"

#define FIXTURE     "test/fixtures/mock/gc/game.ciso"
#define EXTRACT_DIR "test/out/ciso"

#define EXPECTED_GAME_ID "SIPHON"
#define EXPECTED_FORMAT  "CISO"

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

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static int test_inspect_ok(void) {
    SiphonDiscInfo info;
    SiphonError err = siphon_disc_inspect(FIXTURE, &info, silent_log, NULL);
    if (err != SIPHON_OK) {
        fprintf(stderr, "inspect returned %d, expected SIPHON_OK\n", err);
        return 1;
    }
    return 0;
}

static int test_inspect_format(void) {
    SiphonDiscInfo info;
    siphon_disc_inspect(FIXTURE, &info, silent_log, NULL);
    if (strcmp(info.format, EXPECTED_FORMAT) != 0) {
        fprintf(stderr, "format: got '%s', expected '%s'\n",
                info.format, EXPECTED_FORMAT);
        return 1;
    }
    return 0;
}

static int test_inspect_game_id(void) {
    SiphonDiscInfo info;
    siphon_disc_inspect(FIXTURE, &info, silent_log, NULL);
    if (strncmp(info.game_id, EXPECTED_GAME_ID, strlen(EXPECTED_GAME_ID)) != 0) {
        fprintf(stderr, "game_id: got '%.6s', expected '%s'\n",
                info.game_id, EXPECTED_GAME_ID);
        return 1;
    }
    return 0;
}

static int test_inspect_entry_count(void) {
    SiphonDiscInfo info;
    siphon_disc_inspect(FIXTURE, &info, silent_log, NULL);
    if (info.entry_count != 0) {
        fprintf(stderr, "entry_count: got %d, expected 0 (empty fixture)\n",
                info.entry_count);
        return 1;
    }
    return 0;
}

static int test_extract_ok(void) {
    SiphonError err = siphon_disc_extract(
        FIXTURE, EXTRACT_DIR,
        NULL, 0,
        silent_log, NULL
    );
    if (err != SIPHON_OK) {
        fprintf(stderr, "extract returned %d, expected SIPHON_OK\n", err);
        return 1;
    }
    return 0;
}

static int test_extract_id_match(void) {
    const char* ids[] = { EXPECTED_GAME_ID };
    SiphonError err = siphon_disc_extract(
        FIXTURE, EXTRACT_DIR,
        ids, 1,
        silent_log, NULL
    );
    if (err != SIPHON_OK) {
        fprintf(stderr, "extract with matching ID returned %d\n", err);
        return 1;
    }
    return 0;
}

static int test_extract_id_mismatch(void) {
    const char* ids[] = { "NOTMATCH" };
    SiphonError err = siphon_disc_extract(
        FIXTURE, EXTRACT_DIR,
        ids, 1,
        silent_log, NULL
    );
    if (err != SIPHON_ERR_ID_MISMATCH) {
        fprintf(stderr, "expected SIPHON_ERR_ID_MISMATCH, got %d\n", err);
        return 1;
    }
    return 0;
}

static int test_inspect_bad_file(void) {
    SiphonDiscInfo info;
    SiphonError err = siphon_disc_inspect(
        "test/fixtures/mock/garbage.bin",
        &info, silent_log, NULL
    );
    if (err == SIPHON_OK) {
        fprintf(stderr, "bad file accepted as SIPHON_OK; should have errored\n");
        return 1;
    }
    return 0;
}

static int test_log_on_error(void) {
    LogCapture lc = {0};
    SiphonDiscInfo info;
    siphon_disc_inspect(
        "test/fixtures/mock/garbage.bin",
        &info, capture_log, &lc
    );
    if (lc.len == 0) {
        fprintf(stderr, "no log output on bad-file inspect\n");
        return 1;
    }
    return 0;
}

int main(void) {
    int failures = 0;

#define RUN(t) do { \
    if ((t)() != 0) { failures++; fprintf(stderr, "FAIL " #t "\n"); } \
} while (0)

    RUN(test_inspect_ok);
    RUN(test_inspect_format);
    RUN(test_inspect_game_id);
    RUN(test_inspect_entry_count);
    RUN(test_extract_ok);
    RUN(test_extract_id_match);
    RUN(test_extract_id_mismatch);
    RUN(test_inspect_bad_file);
    RUN(test_log_on_error);

#undef RUN

    if (failures == 0)
        printf("all tests passed\n");
    return failures;
}