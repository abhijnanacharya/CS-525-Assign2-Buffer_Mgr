#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
extern RC shutdownBufferPoolHelper(BM_BufferPool *const bm)
{
    // write dirty pages back to disk
    RC rcCode = forceFlushPool(bm);
    if (rcCode != RC_OK){
        RC_message="Force Flush Failed";
        printError(*RC_message);
    }
        return rcCode;
    // free up resources
    Buffer *bufferPtr = bm->mgmtData;
    Frame *framePtr = bufferPtr->head;

    while (framePtr != bufferPtr->tail)
    {
        framePtr = framePtr->next;
        free(bufferPtr->head);
        bufferPtr->head = framePtr;
    }
    free(bufferPtr->tail);
    free(bufferPtr);

    bm->numPages = 0;
    bm->pageFile = NULL;
    bm->mgmtData = NULL;
    return RC_OK;
}

extern RC forceFlushPoolHelper(BM_BufferPool *const bm)
{
    // Write all dirty pages to disk
    // Check for pinned pages and DO NOT ADD THEM TO DISK
    
    Buffer *bufferPtr = bm->mgmtData;

    SM_FileHandle fileHandler;
    RC rcCode = openPageFile(bm->pageFile, &fileHandler);
    if (rcCode != RC_OK)
        return rcCode;

    Frame *framePtr = bufferPtr->head;
    do
    {
        // If the page is dirty, write it to disk
        if (framePtr->dirtyFlag)
        {
            rcCode = writeBlock(framePtr->currpage, &fileHandler, framePtr->data);
            if (rcCode != RC_OK){
                RC_message="ERR:FAILED TO WRITE DIRTY PAGE\n";
                printError(*RC_message);
                return rcCode;
            }
            // Reset dirty flag after writing to disk
            framePtr->dirtyFlag = false;
            bufferPtr->writeCount++;
        }
        framePtr = framePtr->next;
    } while (framePtr != bufferPtr->head);

    closePageFile(&fileHandler);
    return RC_OK;
}

extern RC initBufferPoolHelper(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData)
{
    // error check
    if (numPages <= 0) // input check
        return RC_WRITE_FAILED;
    // init bf:bookkeeping data
    Buffer *bufferPtr = malloc(sizeof(Buffer));

    if (bufferPtr == NULL)
        return RC_WRITE_FAILED;
    bufferPtr->numFrames = numPages;
    bufferPtr->stratData = stratData;
    bufferPtr->readCount = 0;
    bufferPtr->writeCount = 0;
    // create list
    Frame *newFrame = malloc(sizeof(Frame));
    Statlist *statPtr = malloc(sizeof(Statlist));
    if (newFrame == NULL)
        return RC_WRITE_FAILED;
    newFrame->currpage = NO_PAGE;
    newFrame->refbit = false;
    newFrame->dirtyFlag = false;
    newFrame->fixCount = 0;
    statPtr->fpt = newFrame;
    memset(newFrame->data, '\0', PAGE_SIZE);

    bufferPtr->head = newFrame;
    bufferPtr->statListHead = statPtr;

    int i = 1; // Start the loop counter at 1
    while (i < numPages)
    { 
        Frame *pnew = malloc(sizeof(Frame));
        Statlist *snew = malloc(sizeof(Statlist));
        if (pnew == NULL)
            return RC_WRITE_FAILED;
        pnew->currpage = NO_PAGE;
        pnew->dirtyFlag = false;
        pnew->refbit = false;
        pnew->fixCount = 0;
        memset(pnew->data, '\0', PAGE_SIZE);

        snew->fpt = pnew;
        statPtr->next = snew;
        statPtr = snew;

        newFrame->next = pnew;
        pnew->prev = newFrame;
        newFrame = pnew;
        
        i++; // Increment the loop counter
    }
    statPtr->next = NULL;
    bufferPtr->tail = newFrame;
    bufferPtr->pointer = bufferPtr->head;

    // circular list for clock
    bufferPtr->tail->next = bufferPtr->head;
    bufferPtr->head->prev = bufferPtr->tail;

    // init bm
    bm->numPages = numPages;
    bm->pageFile = (char *)pageFileName;
    bm->strategy = strategy;
    bm->mgmtData = bufferPtr;

    return RC_OK;
}