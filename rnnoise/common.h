

#ifndef COMMON_H
#define COMMON_H

#include "stdlib.h"
#include "string.h"

#define RNN_INLINE inline
#define OPUS_INLINE inline

#define CHANNEL_NUM  1                                           // 声道数
#define SAMPLE_RATE 8000                                      // 每秒采样率
#define FRAME_SIZE  (SAMPLE_RATE/100)
#define SAMPLE_BITS 16                                           // 采样位深
#define AUDIO_DATA_BLOCK_SIZE (FRAME_SIZE * SAMPLE_BITS / 8 * 1)  // 缓存数据块大小 = 采样率*位深/2*秒（字节）                                
#define BUFFER_NUM 10    
/** RNNoise wrapper for malloc(). To do your own dynamic allocation, all you need t
o do is replace this function and rnnoise_free */
#ifndef OVERRIDE_RNNOISE_ALLOC
static RNN_INLINE void *rnnoise_alloc (size_t size)
{
   return malloc(size);
}
#endif

/** RNNoise wrapper for free(). To do your own dynamic allocation, all you need to do is replace this function and rnnoise_alloc */
#ifndef OVERRIDE_RNNOISE_FREE
static RNN_INLINE void rnnoise_free (void *ptr)
{
   free(ptr);
}
#endif

/** Copy n elements from src to dst. The 0* term provides compile-time type checking  */
#ifndef OVERRIDE_RNN_COPY
#define RNN_COPY(dst, src, n) (memcpy((dst), (src), (n)*sizeof(*(dst)) + 0*((dst)-(src)) ))
#endif

/** Copy n elements from src to dst, allowing overlapping regions. The 0* term
    provides compile-time type checking */
#ifndef OVERRIDE_RNN_MOVE
#define RNN_MOVE(dst, src, n) (memmove((dst), (src), (n)*sizeof(*(dst)) + 0*((dst)-(src)) ))
#endif

/** Set n elements of dst to zero */
#ifndef OVERRIDE_RNN_CLEAR
#define RNN_CLEAR(dst, n) (memset((dst), 0, (n)*sizeof(*(dst))))
#endif
#endif
