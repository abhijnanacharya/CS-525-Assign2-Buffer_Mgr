#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "buffer_mgr.h"
#include "replacement_mgr_strat.h"
#include "storage_mgr.h"


extern void initializeVariables(Frame *newFrame, Frame *frame, int index){
	newFrame[index].bm_PageHandle.data = frame->bm_PageHandle.data;
	newFrame[index].bm_PageHandle.pageNum = frame->bm_PageHandle.pageNum;
	newFrame[index].dirtyCount = frame->dirtyCount;
	newFrame[index].fixCount = frame->fixCount;
}

extern void FIFO(BM_BufferPool *const bm, Frame *frame, int noOfPagesRead, int noOfPagesWrite, int maxBufferSize)
{
	Frame *pageFrame = (Frame *) bm->mgmtData;
	
	int frontIndex = noOfPagesRead % maxBufferSize;

	int i = 0;
	while(i < maxBufferSize)
	{
		if(pageFrame[frontIndex].fixCount == 0)
		{
			if(pageFrame[frontIndex].dirtyCount == 1)
			{
				SM_FileHandle fh;
				openPageFile(bm->pageFile, &fh);
				writeBlock(pageFrame[frontIndex].bm_PageHandle.pageNum, &fh, pageFrame[frontIndex].bm_PageHandle.data);
				
				noOfPagesWrite += 1;
			}
			
			initializeVariables(pageFrame, frame, frontIndex);
			break;
		}
		else
		{
			if (frontIndex % maxBufferSize == 0){
				frontIndex = 0;
			} else {
				frontIndex++;
			}
		}

		i++;
	}
}

extern void LRU(BM_BufferPool *const bm, Frame *frame, int maxBufferSize, int noOfPagesWrite)
{	
	Frame *pageFrame = (Frame *) bm->mgmtData;
	int leastHitIdx = 0;
	int leastHitRef = 0;

	int i = 0;

	while(i < maxBufferSize)
	{
		if(pageFrame[i].fixCount == 0)
		{
			leastHitIdx = i;
			leastHitRef = pageFrame[i].hit;
			break;
		}

		i++;
	}	

	for(int i = leastHitIdx + 1; i < maxBufferSize; i++)
	{
		if(pageFrame[i].hit < leastHitRef)
		{
			leastHitIdx = i;
			leastHitRef = pageFrame[i].hit;
		}
	}

	if(pageFrame[leastHitIdx].dirtyCount == 1)
	{
		SM_FileHandle fh;
		openPageFile(bm->pageFile, &fh);
		writeBlock(pageFrame[leastHitIdx].bm_PageHandle.pageNum, &fh, pageFrame[leastHitIdx].bm_PageHandle.data);
		
		noOfPagesWrite++;
	}
	
	initializeVariables(pageFrame, frame, leastHitIdx);
}


extern void CLOCK(BM_BufferPool *const bm, Frame *page, int clockPointer, int maxBufferSize, int noOfPagesWrite)
{	
	Frame *pageFrame = (Frame *) bm->mgmtData;
	while(true)
	{
		if (clockPointer % maxBufferSize == 0){
			clockPointer = 0;
		}

		if(pageFrame[clockPointer].hit == 0)
		{
			if(pageFrame[clockPointer].dirtyCount == 1)
			{
				SM_FileHandle fh;
				openPageFile(bm->pageFile, &fh);
				writeBlock(pageFrame[clockPointer].bm_PageHandle.pageNum, &fh, pageFrame[clockPointer].bm_PageHandle.data);
				
				noOfPagesWrite += 1;
			}
			
			initializeVariables(pageFrame, page, clockPointer);

			clockPointer += 1;
			break;	
		}
		else
		{
			clockPointer += 1;
			pageFrame[clockPointer].hit = 0;		
		}
	}
}