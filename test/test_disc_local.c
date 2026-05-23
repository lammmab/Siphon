#include "siphon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void silent(void* u, const char* m) { (void)u; (void)m; }

int main(void) {
    const char* path = getenv("SIPHON_TEST_DISC");
    if (!path || !*path) {
        printf("skipped: set SIPHON_TEST_DISC=<disc image> to run\n");
        return 77;
    }

    SiphonDiscInfo info;
    SiphonError err = siphon_disc_inspect(path, &info, silent, NULL);
    if (err != SIPHON_OK) {
        fprintf(stderr, "inspect(%s) returned %d\n", path, err);
        return 1;
    }
    if (info.format[0] == '\0') { fprintf(stderr, "empty format\n"); return 1; }
    if (info.game_id[0] == '\0') { fprintf(stderr, "empty game id\n"); return 1; }

    const char* expid = getenv("SIPHON_TEST_DISC_ID");
    if (expid && *expid && strncmp(info.game_id, expid, strlen(expid)) != 0) {
        fprintf(stderr, "game id: got '%.6s', expected '%s'\n", info.game_id, expid);
        return 1;
    }

    const char* outdir = getenv("SIPHON_TEST_OUT");
    if (!outdir || !*outdir) outdir = "siphon_test_disc_out";
    err = siphon_disc_extract(path, outdir, NULL, 0, silent, NULL);
    if (err != SIPHON_OK) {
        fprintf(stderr, "extract returned %d\n", err);
        return 1;
    }

    printf("ok: format=%s id=%.6s entries=%d\n", info.format, info.game_id, info.entry_count);
    return 0;
}
