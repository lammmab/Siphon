#ifndef SIPHON_LZMA_H
#define SIPHON_LZMA_H
#include <stddef.h>
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif
typedef unsigned char Byte;
typedef unsigned short UInt16;
typedef unsigned int UInt32;
typedef unsigned long long UInt64;
typedef size_t SizeT;
typedef int Bool;
#define True 1
#define False 0
typedef int SRes;
#define SZ_OK 0
#define SZ_ERROR_DATA 1
#define SZ_ERROR_MEM 2
#define SZ_ERROR_CRC 3
#define SZ_ERROR_UNSUPPORTED 4
#define SZ_ERROR_PARAM 5
#define SZ_ERROR_INPUT_EOF 6
#define SZ_ERROR_OUTPUT_EOF 7
#define SZ_ERROR_READ 8
#define SZ_ERROR_WRITE 9
#define SZ_ERROR_PROGRESS 10
#define SZ_ERROR_FAIL 11
#define MY_FAST_CALL
#define RINOK(x) { int __result__ = (x); if (__result__ != 0) return __result__; }
typedef struct ISzAlloc ISzAlloc;
typedef const ISzAlloc * ISzAllocPtr;
struct ISzAlloc { void *(*Alloc)(ISzAllocPtr p, size_t size); void (*Free)(ISzAllocPtr p, void *address); };
#define ISzAlloc_Alloc(p, size) (p)->Alloc(p, size)
#define ISzAlloc_Free(p, a) (p)->Free(p, a)

EXTERN_C_BEGIN

typedef
#ifdef _LZMA_PROB32
  UInt32
#else
  UInt16
#endif
  CLzmaProb;

#define LZMA_PROPS_SIZE 5

typedef struct _CLzmaProps
{
  Byte lc;
  Byte lp;
  Byte pb;
  Byte _pad_;
  UInt32 dicSize;
} CLzmaProps;

SRes LzmaProps_Decode(CLzmaProps *p, const Byte *data, unsigned size);

#define LZMA_REQUIRED_INPUT_MAX 20

typedef struct
{

  CLzmaProps prop;
  CLzmaProb *probs;
  CLzmaProb *probs_1664;
  Byte *dic;
  SizeT dicBufSize;
  SizeT dicPos;
  const Byte *buf;
  UInt32 range;
  UInt32 code;
  UInt32 processedPos;
  UInt32 checkDicSize;
  UInt32 reps[4];
  UInt32 state;
  UInt32 remainLen;

  UInt32 numProbs;
  unsigned tempBufSize;
  Byte tempBuf[LZMA_REQUIRED_INPUT_MAX];
} CLzmaDec;

#define LzmaDec_Construct(p) { (p)->dic = NULL; (p)->probs = NULL; }

void LzmaDec_Init(CLzmaDec *p);

typedef enum
{
  LZMA_FINISH_ANY,
  LZMA_FINISH_END
} ELzmaFinishMode;

typedef enum
{
  LZMA_STATUS_NOT_SPECIFIED,
  LZMA_STATUS_FINISHED_WITH_MARK,
  LZMA_STATUS_NOT_FINISHED,
  LZMA_STATUS_NEEDS_MORE_INPUT,
  LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK
} ELzmaStatus;

SRes LzmaDec_AllocateProbs(CLzmaDec *p, const Byte *props, unsigned propsSize, ISzAllocPtr alloc);
void LzmaDec_FreeProbs(CLzmaDec *p, ISzAllocPtr alloc);

SRes LzmaDec_Allocate(CLzmaDec *p, const Byte *props, unsigned propsSize, ISzAllocPtr alloc);
void LzmaDec_Free(CLzmaDec *p, ISzAllocPtr alloc);

SRes LzmaDec_DecodeToDic(CLzmaDec *p, SizeT dicLimit,
    const Byte *src, SizeT *srcLen, ELzmaFinishMode finishMode, ELzmaStatus *status);

SRes LzmaDec_DecodeToBuf(CLzmaDec *p, Byte *dest, SizeT *destLen,
    const Byte *src, SizeT *srcLen, ELzmaFinishMode finishMode, ELzmaStatus *status);

SRes LzmaDecode(Byte *dest, SizeT *destLen, const Byte *src, SizeT *srcLen,
    const Byte *propData, unsigned propSize, ELzmaFinishMode finishMode,
    ELzmaStatus *status, ISzAllocPtr alloc);

EXTERN_C_END

#endif
