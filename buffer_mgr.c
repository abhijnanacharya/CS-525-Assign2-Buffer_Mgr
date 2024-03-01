#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "buffer_mgr.h"
#include "replacement_mgr_strat.h"
#include "storage_mgr.h"

int maxBufferSize = 0;
int numberOfPagesRead = 0;
int numberOfPagesWritten = 0;
int hit = 0;
int clockPointer = 0;

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData)
{
  // Initialize return code to indicate success by default
  RC returnCode = RC_OK;

  // Check if buffer pool pointer is null
  if (!bm) {
    // Initialize buffer pool properties
    bm->numPages = numPages;         // Set number of pages
    bm->strategy = strategy;         // Set replacement strategy
    bm->pageFile = (char *) pageFileName;  // Set page file name
    maxBufferSize = numPages;        // Set maximum buffer size
  }

  // Allocate dynamic memory for frames stored as an array
  Frame *frame = malloc(numPages * sizeof(Frame));

  // Initialize each frame in the buffer pool
  int currVal = 0;
  while (currVal < maxBufferSize) {
    // Initialize frame properties
    frame[currVal].bm_PageHandle.data = NULL; // Set data pointer to null
    frame[currVal].bm_PageHandle.pageNum = -1; // Set page number to -1 (invalid)
    frame[currVal].dirtyCount = 0;   // Initialize dirty count to 0
    frame[currVal].fixCount = 0;     // Initialize fix count to 0
    frame[currVal].hit = 0;          // Initialize hit count to 0

    // Move to the next frame
    currVal++;
  }

  // Set buffer pool management data to point to the allocated frames
  bm->mgmtData = frame;

  // Return the initialized buffer pool status
  return returnCode;
}