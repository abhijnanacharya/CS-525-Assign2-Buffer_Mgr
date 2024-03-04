#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <string.h>
#include <stdlib.h>
#include "replacement_mgr_strat.h"
#include "buffer_mgr_helper.h"

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData)
{
    return initBufferPoolHelper(bm, pageFileName, numPages, strategy, stratData);
}

RC shutdownBufferPool(BM_BufferPool *const bm)
{
    return shutdownBufferPoolHelper(bm);
}

RC forceFlushPool(BM_BufferPool *const bm)
{
    return forceFlushPoolHelper(bm);
}

RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    // Buffer *bufferPtr = bm->mgmtData;
    // Frame *framePtr = bufferPtr->head;
    // Check if bm pointer is not NULL
    if (bm == NULL)
    {
        RC_message = "ERROR: Buffer pool pointer is NULL!";
        printError(*RC_message);
        return RC_BUFFER_NOT_INIT;
    }

    // Retrieve buffer pointer
    Buffer *bufferPtr = bm->mgmtData;

    // Check if buffer pointer is not NULL
    if (bufferPtr == NULL)
    {
        RC_message = "ERROR: MANAGEMENT DATA IS NULL!";
        printError(*RC_message);
        return RC_MGMT_POINTER_NULL;
    }

    // Retrieve frame pointer
    Frame *framePtr = bufferPtr->head;

    // Check if frame pointer is not NULL
    if (framePtr == NULL)
    {
        RC_message = "ERROR: Frame pointer is NULL!";
        printError(*RC_message);
        return RC_FRAME_POINTER_FAILURE;
    }

    do
    {
        framePtr = framePtr->next;
        if (framePtr == bufferPtr->head)
        {
            RC_message = "ERR:TRYING TO READ NON_EXISTENT PAGE!!!";
            printError(*RC_message);
            return RC_READ_NON_EXISTING_PAGE;
        }

    } while (framePtr->currpage != page->pageNum);

    framePtr->dirtyFlag = !false;
    return RC_OK;
}

RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    // Retrieve buffer and frame pointers
    Buffer *bufferPtr = bm->mgmtData;
    Frame *framePtr = bufferPtr->head;

    // Search for the page in the buffer pool
    for (int i = 0; i <= 1000; i++)
    {
        if (framePtr->currpage == page->pageNum)
        {
            break;
        }
        framePtr = framePtr->next;
        // If we've traversed the entire buffer without finding the page, return error
        if (framePtr == bufferPtr->head)
        {
            RC_message = "ERR: READING NOT EXISTENT PAGE!!";
            printError(*RC_message);
        }
    }

    // If the page is found, decrement its fix count
    if (framePtr->fixCount > 0)
    {
        framePtr->fixCount = framePtr->fixCount - 1;
        // If fix count reaches 0, mark the page as unpinned
        if (framePtr->fixCount == 0)
            framePtr->refbit = false;
    }
    else
    {
        // Handle the case where the page is not found (should not happen)
        RC_message = "ERROR: ATTEMPTING TO UNPIN NON-EXISTENT PAGE!";
        printError(*RC_message);
        return RC_READ_NON_EXISTING_PAGE;
    }

    return RC_OK;
}

RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    // Open the page file
    SM_FileHandle fileHandler;
    RC rcCode = openPageFile(bm->pageFile, &fileHandler);
    if (rcCode != RC_OK)
    {
        RC_message = "ERR: FILE NOT FOUND";
        printError(*RC_message);
        return RC_FILE_NOT_FOUND; // Return error if file not found
    }

    // Write the page to disk
    rcCode = writeBlock(page->pageNum, &fileHandler, page->data);
    if (rcCode != RC_OK)
    {
        // Print error message and close the file if write fails
        RC_message = "ERR:ERROR WRITING PAGE TO FILE";
        printError(*RC_message);
        closePageFile(&fileHandler);
        return RC_WRITE_FAILED;
    }

    // Increment write count and close the page file
    ((Buffer *)bm->mgmtData)->writeCount = ((Buffer *)bm->mgmtData)->writeCount + 1;
    closePageFile(&fileHandler);

    return RC_OK; // Return success
}
RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page,
           const PageNumber pageNum)
{
    if (pageNum < 0)
    {
        RC_message = "ERR: INVALID PAGE NUMBER";
        printError(*RC_message);
        return RC_INVALID_PAGE_VALUE;
    }

    int strategy = bm->strategy;
    if (strategy == RS_FIFO)
        return FIFO(bm, page, pageNum, false);
    else if (strategy == RS_LRU)
        return LRU(bm, page, pageNum);
    else if (strategy == RS_CLOCK)
        return CLOCK(bm, page, pageNum);
    else if (strategy == LRUK)
        return LRUK(bm, page, pageNum);
    else
    {
        RC_message = "ERR:UNKNOWN STRATEGY!!";
        printError(*RC_message);
        return RC_IM_KEY_NOT_FOUND;
    }
}

PageNumber *getFrameContents(BM_BufferPool *const bm)
{
    int pageCount = bm->numPages;
    // PageNumber *pgPtr = calloc(bm->numPages, sizeof(int));
    PageNumber *pgPtr = (PageNumber *)malloc(bm->numPages * sizeof(int));
    if (pgPtr != NULL)
    {
        memset(pgPtr, 0, bm->numPages * sizeof(int));
    }
    else
    {
        RC_message = "ERR: PAGE POINTER MEM ALLOC FAILURE";
        printError(*RC_message);
        return RC_MEM_ALLOC_FAILED;
    }
    Buffer *bufferPtr = bm->mgmtData;
    Statlist *statListPtr = bufferPtr->statListHead;
    int idx = 0;
    while (idx < pageCount)
    {
        pgPtr[idx] = statListPtr->fpt->currpage;
        statListPtr = statListPtr->next;
        idx++;
    }
    return pgPtr;
}

bool *getDirtyFlags(BM_BufferPool *const bm)
{
    bool *pgPtr = calloc(bm->numPages, sizeof(bool));
    int pageC = (bm->numPages); // Set page count
    Buffer *bufferPtr = bm->mgmtData;
    Statlist *spt = bufferPtr->statListHead;
    int idx = 0;
    while (idx < pageC)
    {
        if (spt->fpt->dirtyFlag)
            pgPtr[idx] = true;
        spt = spt->next;
        idx++;
    }
    return pgPtr;
}

int *getFixCounts(BM_BufferPool *const bm)
{
    PageNumber *pgNoPtr = calloc(bm->numPages, sizeof(int));

    // Check if memory allocation was successful
    if (pgNoPtr == NULL)
    {
        RC_message = "ERROR: MEM ALLOC FAILED!";
        printError(*RC_message);
        return RC_MEM_ALLOC_FAILED;
    }
    Buffer *bufferPtr = bm->mgmtData;
    int pageC = (bm->numPages); // Set page count
    Statlist *statPtr = bufferPtr->statListHead;
    int i = 0;
    while (i < pageC)
    {
        pgNoPtr[i] = statPtr->fpt->fixCount;
        statPtr = statPtr->next;
        i++;
    }
    return pgNoPtr;
}
int getNumReadIO(BM_BufferPool *const bm)
{
    return ((Buffer *)bm->mgmtData)->readCount;
}

int getNumWriteIO(BM_BufferPool *const bm)
{
    return ((Buffer *)bm->mgmtData)->writeCount;
}