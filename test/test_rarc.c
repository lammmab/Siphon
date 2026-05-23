#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "siphon.h"
#include <confluence/macros.h>

#define FIXTURE_ARC   "test/fixtures/minimal.arc"
#define FIXTURE_INNER "tex/sw_rope.bti"
#define EXTRACT_DIR   "test/out/rarc"
#define COPY_OUT      "test/out/rarc_copy/sw_rope.bti"
#define TMP_ARC       "test/out/rarc_tmp/minimal_tmp.arc"

typedef struct { char buf[2048]; size_t len; } LogCapture;

static void capture_log(void* userdata, const char* msg) {
    LogCapture* lc = (LogCapture*)userdata;
    size_t room = sizeof(lc->buf) - lc->len - 1;
    if (!room) return;
    size_t n = strlen(msg); if (n > room) n = room;
    memcpy(lc->buf + lc->len, msg, n);
    lc->len += n;
    lc->buf[lc->len] = '\0';
}
static void silent_log(void* u, const char* m) { (void)u; (void)m; }

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f); return 1;
}

static long file_size(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END); long sz = ftell(f);
    fclose(f); return sz;
}

static void wr32(unsigned char* p, unsigned int v) {
    p[0]=(v>>24)&0xFF; p[1]=(v>>16)&0xFF; p[2]=(v>>8)&0xFF; p[3]=v&0xFF;
}
static void wr16(unsigned char* p, unsigned short v) {
    p[0]=(v>>8)&0xFF; p[1]=v&0xFF;
}

static int write_minimal_rarc(const char* path) {
    const unsigned int HDR=0,FST_INFO=0x20,DIR_TBL=0x40,FILE_TBL=0x50,
                       STR_TBL=0xA0,DATA=0xB4,TOTAL=0xB8;
    const unsigned int STR_ROOT=0,STR_DOT=5,STR_DOTDOT=7,STR_FILE=0xA,STR_LEN=0x14;

    unsigned char* buf = (unsigned char*)calloc(1, TOTAL);
    if (!buf) return -1;

    memcpy(buf+HDR,    "RARC", 4);
    wr32(buf+HDR+0x04, TOTAL);
    wr32(buf+HDR+0x08, 0x20);
    wr32(buf+HDR+0x0C, DATA-0x20);
    wr32(buf+HDR+0x10, 0x04);
    wr32(buf+HDR+0x14, 0x04);

    unsigned char* fst = buf+FST_INFO;
    wr32(fst+0x00, 1);
    wr32(fst+0x04, DIR_TBL-FST_INFO);
    wr32(fst+0x08, 4);
    wr32(fst+0x0C, FILE_TBL-FST_INFO);
    wr32(fst+0x10, STR_LEN);
    wr32(fst+0x14, STR_TBL-FST_INFO);
    wr16(fst+0x18, 1);

    unsigned char* dir = buf+DIR_TBL;
    wr32(dir+0x00, 0xFFFFFFFF);
    wr32(dir+0x04, 0x52415243);
    wr16(dir+0x08, STR_ROOT);
    wr16(dir+0x0A, 4);
    wr32(dir+0x0C, 0);

    unsigned char* fe = buf+FILE_TBL;
    wr16(fe+0x00,0xFFFF); wr16(fe+0x04,0x0200); wr16(fe+0x06,STR_DOT);   wr32(fe+0x08,0); fe+=0x14;
    wr16(fe+0x00,0xFFFF); wr16(fe+0x04,0x0200); wr16(fe+0x06,STR_DOTDOT); wr32(fe+0x08,0xFFFFFFFF); fe+=0x14;
    wr16(fe+0x00,0xFFFF); wr16(fe+0x04,0x0200); wr16(fe+0x06,STR_ROOT);  wr32(fe+0x08,0); fe+=0x14;
    wr16(fe+0x00,0x0000); wr16(fe+0x04,0x1100); wr16(fe+0x06,STR_FILE);  wr32(fe+0x08,0); wr32(fe+0x0C,4);

    unsigned char* st = buf+STR_TBL;
    memcpy(st+STR_ROOT,"ROOT",4);     st[STR_ROOT+4]='\0';
    memcpy(st+STR_DOT,".",1);         st[STR_DOT+1]='\0';
    memcpy(st+STR_DOTDOT,"..",2);     st[STR_DOTDOT+2]='\0';
    memcpy(st+STR_FILE,"file.bin",8); st[STR_FILE+8]='\0';

    buf[DATA]=0xDE; buf[DATA+1]=0xAD; buf[DATA+2]=0xBE; buf[DATA+3]=0xEF;

    MKDIR_ONE("test/out"); MKDIR_ONE("test/out/rarc_tmp");
    FILE* f = fopen(path, "wb");
    if (!f) { free(buf); return -1; }
    int ok = (fwrite(buf, 1, TOTAL, f) == TOTAL);
    fclose(f); free(buf);
    return ok ? 0 : -1;
}

static int test_extract_ok(void) {
    if (write_minimal_rarc(TMP_ARC) != 0) {
        fprintf(stderr, "failed to write tmp arc\n"); return 1;
    }
    MKDIR_ONE(EXTRACT_DIR);
    SiphonError err = siphon_arc_extract(TMP_ARC, EXTRACT_DIR, silent_log, NULL);
    if (err != SIPHON_OK) {
        fprintf(stderr, "arc_extract returned %d\n", err); return 1;
    }
    return 0;
}

static int test_extract_all(void) {
    if (!file_exists(FIXTURE_ARC)) {
        fprintf(stderr, "fixture %s not found\n", FIXTURE_ARC); return 1;
    }
    MKDIR_ONE(EXTRACT_DIR);
    SiphonError err = siphon_arc_extract(FIXTURE_ARC, EXTRACT_DIR, silent_log, NULL);
    if (err != SIPHON_OK) {
        fprintf(stderr, "arc_extract returned %d\n", err); return 1;
    }
    if (!file_exists("test/out/rarc/tex/sw_rope.bti")) {
        fprintf(stderr, "tex/sw_rope.bti not extracted\n"); return 1;
    }
    if (file_size("test/out/rarc/tex/sw_rope.bti") != 160) {
        fprintf(stderr, "sw_rope.bti wrong size\n"); return 1;
    }
    return 0;
}

static int test_list_ok(void) {
    if (!file_exists(FIXTURE_ARC)) {
        fprintf(stderr, "fixture %s not found\n", FIXTURE_ARC); return 1;
    }
    LogCapture lc = {0};
    SiphonError err = siphon_arc_list(FIXTURE_ARC, capture_log, &lc);
    if (err != SIPHON_OK) {
        fprintf(stderr, "arc_list returned %d\n", err); return 1;
    }
    if (lc.len == 0) {
        fprintf(stderr, "arc_list produced no output\n"); return 1;
    }
    return 0;
}

static int test_list_contains_known_file(void) {
    if (!file_exists(FIXTURE_ARC)) {
        fprintf(stderr, "fixture %s not found\n", FIXTURE_ARC); return 1;
    }
    LogCapture lc = {0};
    siphon_arc_list(FIXTURE_ARC, capture_log, &lc);
    if (!strstr(lc.buf, "sw_rope.bti")) {
        fprintf(stderr, "arc_list output missing 'sw_rope.bti'\n"); return 1;
    }
    return 0;
}

static int test_copy_single_file(void) {
    if (!file_exists(FIXTURE_ARC)) {
        fprintf(stderr, "fixture %s not found\n", FIXTURE_ARC); return 1;
    }
    MKDIR_ONE("test/out"); MKDIR_ONE("test/out/rarc_copy");
    SiphonError err = siphon_arc_copy(
        FIXTURE_ARC, FIXTURE_INNER, COPY_OUT, silent_log, NULL
    );
    if (err != SIPHON_OK) {
        fprintf(stderr, "arc_copy returned %d\n", err); return 1;
    }
    if (!file_exists(COPY_OUT)) {
        fprintf(stderr, "%s not created\n", COPY_OUT); return 1;
    }
    if (file_size(COPY_OUT) != 160) {
        fprintf(stderr, "copied file wrong size\n"); return 1;
    }
    return 0;
}

static int test_copy_missing_inner(void) {
    if (!file_exists(FIXTURE_ARC)) {
        fprintf(stderr, "fixture %s not found\n", FIXTURE_ARC); return 1;
    }
    SiphonError err = siphon_arc_copy(
        FIXTURE_ARC, "does/not/exist.bin", COPY_OUT, silent_log, NULL
    );
    if (err != SIPHON_ERR_NOT_FOUND) {
        fprintf(stderr, "expected SIPHON_ERR_NOT_FOUND, got %d\n", err); return 1;
    }
    return 0;
}

static int test_bad_file(void) {
    SiphonError err = siphon_arc_extract(
        "test/fixtures/mock/garbage.bin", EXTRACT_DIR, silent_log, NULL
    );
    if (err == SIPHON_OK) {
        fprintf(stderr, "bad file accepted as SIPHON_OK\n"); return 1;
    }
    return 0;
}

int main(void) {
    int failures = 0;

#define RUN(t) do { \
    if ((t)() != 0) { failures++; fprintf(stderr, "FAIL " #t "\n"); } \
} while (0)

    RUN(test_extract_ok);
    RUN(test_extract_all);
    RUN(test_list_ok);
    RUN(test_list_contains_known_file);
    RUN(test_copy_single_file);
    RUN(test_copy_missing_inner);
    RUN(test_bad_file);

#undef RUN

    if (failures == 0) printf("all tests passed\n");
    return failures;
}