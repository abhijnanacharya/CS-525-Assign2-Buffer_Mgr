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
    frame[currVal].currentPage = NO_PAGE; // setting currentPage to NO_PAGE
    frame[currVal].nextFrame = NULL; // Initialize next frame pointer to NULL
    frame[currVal].isDirty = false; // setting isDirty to false
    // Move to the next frame
    currVal++;
  }

  // Set buffer pool management data to point to the allocated frames
  bm->mgmtData = frame;

  // Return the initialized buffer pool status
  return returnCode;
}

extern RC shutdownBufferPool(BM_BufferPool *const bm)

/*a function named shutdownBufferPool that takes a BM_BufferPool pointer (bm)
 as its argument and returns an RC*/
{
    Frame *pageFrame = (Frame *)bm->mgmtData;/*declaring a pointer to a Frame named pageFrame and assigns it the value of bm->mgmtData*/

    //calling a function named forceFlushPool passing the bm pointer as an argument.
    forceFlushPool(bm);
    int i = 0;
    switch (i)
    {
        case 0:
            while (i < maxBufferSize)
            {

                /*below condition checks if the fix count of the current page frame is not equal to zero.
                 If any page frame has a non-zero fix count, it means that some client is currently using that page,
                 so the function returns RC_PAGES_IN_BUFFER.*/

                if (pageFrame[i].fixCount != 0)
                {
                    return RC_PAGES_IN_BUFFER;
                }
                i++;
            }
            break;
    }
    free(pageFrame);
    bm->mgmtData = NULL;/*t sets the mgmtData field of the buffer pool structure pointed to by bm to NULL.
                        This indicates that the buffer pool is now empty.*/
    return RC_OK;
}



/*Function flushframe is use  to flush a single frame. and it will Within the framework of a buffer pool
 *  management system, the flushFrame function is intended to write the contents of a single 
 * frame to disk. Using openPageFile, it first opens the page file linked to the buffer pool. 
 * The data from the given frame (pageFrame) is then written to the matching page number (pageNum)
 *  in the file using writeBlock. It designates the frame as clean by changing its dirtyCount field to 0,
 *  indicating that the data has been successfully written to disk. It probably also modifies a counter 
 * (numberOfPagesWritten) to keep track of how many pages have been written to disk. This feature makes
 *  sure that changes made to buffer pool pages are saved to disk, preserving consistency between
 *  in-memory data and persistant memory*/

void flushFrame(BM_BufferPool *const bm, Frame *pageFrame, int pageNum) {
    SM_FileHandle fh;
    openPageFile(bm->pageFile, &fh);
    writeBlock(pageNum, &fh, pageFrame->bm_PageHandle.data);
    pageFrame->dirtyCount = 0;
    numberOfPagesWritten++;
}

/*Revised forceFlushPool function using flushFrame and it is use for 
The forceFlushPool function iterates over each frame in the buffer pool. 
For each frame, if it's not currently in use and has been modified, 
it calls flushFrame to write its data to disk. Finally, the function returns RC_OK,
ensuring all dirty pages are successfully flushed to maintain data consistency.*/

extern RC forceFlushPool(BM_BufferPool *const bm) {
    Frame *pageFrame = (Frame *)bm->mgmtData;
    int i = 0;
    switch (i) {
        case 0:
            while (i < maxBufferSize) {
                if (pageFrame[i].fixCount == 0 && pageFrame[i].dirtyCount == 1) {
                    flushFrame(bm, &pageFrame[i], pageFrame[i].bm_PageHandle.pageNum);
                }
                i++;
            }
            break;
    }
    return RC_OK;
}


// Buffer Manager Interface Access Pages
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page) {
  // Checking if buffer manager, page file, or page handle pointers are NULL.
    if (bm == NULL || bm->pageFile == NULL || page == NULL) 
        return RC_ERROR;
    
    buffer *b = (buffer*)bm->mgmtData;
    if (b->head == NULL)
        return RC_READ_NON_EXISTING_PAGE; // page not found

    Frame *f = b->head;
    do {
        if (f->currentPage == page->pageNum) {
            f->isDirty = true; // Mark the frame as dirty since the page has been modified
            return RC_OK; 
        }
        f = f->nextFrame;
    } while (f != b->head);

    return RC_READ_NON_EXISTING_PAGE; // specified page was not found
}


RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
    RC returnCode=RC_OK;
    if (bm == NULL || bm->pageFile == NULL || page == NULL) 
    return RC_ERROR;
    buffer *b = (buffer*)bm->mgmtData;
    Frame *f = b->head;
    while (f->currentPage!=page->pageNum){
        f=f->nextFrame;
        if (f==b->head)
            return RC_READ_NON_EXISTING_PAGE;
    }
    if (f->fixCount > 0){
        f->fixCount--;
        if (f->fixCount == 0)
            f->referenceBit = false;
    }
    else
        return RC_READ_NON_EXISTING_PAGE;

    return returnCode;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    if (bm == NULL || bm->pageFile == NULL || page == NULL) 
    return RC_ERROR;
    buffer *b = (buffer*)bm->mgmtData;
    SM_FileHandle fHandle;
    RC stat;
    // Attempt to open the page file
    stat = openPageFile(bm->pageFile, &fHandle);
    if (stat!=RC_OK) return RC_FILE_NOT_FOUND;
    // Write the page data to disk
    stat = writeBlock(page->pageNum, &fHandle, page->data);
    if (stat!=RC_OK){
        closePageFile(&fHandle);
        return RC_FILE_NOT_FOUND;
    }
    
    b->writeCount++;
    closePageFile(&fHandle); // Close the file handle after successful write
    return RC_OK;
}

