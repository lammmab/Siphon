#include "lzma.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void* a_(ISzAllocPtr p, size_t s) { (void)p; return malloc(s); }
static void  f_(ISzAllocPtr p, void* a) { (void)p; free(a); }

static const unsigned char PROPS[5] = { 93,0,0,1,0 };
static const unsigned char COMP[] = { 0,58,26,8,206,118,199,229,233,214,7,52,195,209,14,191,206,85,225,170,189,224,228,143,152,1,221,141,229,7,84,158,101,37,95,39,58,106,126,180,211,73,3,137,206,212,125,60,255,154,222,54,28,172,17,101,226,202,251,41,137,38,127,3,137,61,7,131,154,253,191,255,240,56,0,0 };

int main(void) {
    const char* phrase = "the quick brown fox jumps over the lazy dog. ";
    size_t pl = strlen(phrase);
    size_t N = 4096;
    unsigned char* expect = (unsigned char*)malloc(N);
    for (size_t i = 0; i < N; i++) expect[i] = (unsigned char)phrase[i % pl];

    unsigned char* out = (unsigned char*)malloc(N + 64);
    ISzAlloc al = { a_, f_ };
    SizeT destLen = N + 64, srcLen = sizeof(COMP);
    ELzmaStatus st;
    SRes r = LzmaDecode(out, &destLen, COMP, &srcLen, PROPS, 5, LZMA_FINISH_ANY, &st, &al);
    if (r != SZ_OK) { fprintf(stderr, "LzmaDecode rc=%d\n", r); return 1; }
    if (destLen != N) { fprintf(stderr, "len: got %lu want %lu\n", (unsigned long)destLen, (unsigned long)N); return 1; }
    if (memcmp(out, expect, N) != 0) { fprintf(stderr, "data mismatch\n"); return 1; }
    printf("lzma ok (%lu -> %lu)\n", (unsigned long)sizeof(COMP), (unsigned long)N);
    return 0;
}
