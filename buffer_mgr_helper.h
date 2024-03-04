#ifndef BUFFER_MGR_HELPER_H
#define BUFFER_MGR_HELPER_H

#include "buffer_mgr.h"

extern RC shutdownBufferPoolHelper(BM_BufferPool *const bm);
extern RC forceFlushPoolHelper(BM_BufferPool *const bm);
extern RC initBufferPoolHelper(BM_BufferPool *const bm, const char *const pageFileName,
                         const int numPages, ReplacementStrategy strategy,
                         void *stratData);
#endif