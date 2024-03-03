#include <stdio.h>
#include <stdlib.h>
#include "storage_mgr.h"
#include "dberror.h"

FILE *fp;

extern void initStorageManager()
{
  fp = NULL;
  printf("**********Storage Manager V0.0.1 initialized**********\n");
}
// Creation of the page file
extern RC createPageFile(char *fName)
{
  RC returnCode = RC_OK;

  // If the file already exists, change it to empty.
  fp = fopen(fName, "w+");

  // If file pointer is NULL, return File Not Found Error.
  if (fp == NULL)
  {
    returnCode = RC_FILE_NOT_FOUND;
    RC_message = "ERR: UNABLE TO LOCATE FILE";
    printError(*RC_message);
    return returnCode;
  }
  size_t blockNum = PAGE_SIZE;
  size_t blockSize = sizeof(char);
  // Allocating memory to PAGE_SIZE element
  SM_PageHandle newblankPage = (SM_PageHandle)calloc(blockNum, blockSize);

  if (newblankPage == NULL)
  {
    // Handle memory allocation failure.
    fclose(fp);
    returnCode = RC_MEM_ALLOC_FAILED;
    RC_message = "ERR: MEMORY ALLOCATION FAILURE";
    printError(*RC_message);
    return returnCode;
  }

  // Writing an empty page to the file
  size_t writeEmptyPageStatus = fwrite(newblankPage, blockSize, blockNum, fp);

  if (writeEmptyPageStatus == 0)
  {
    RC_message = "ERR: FAILED TO WRITE ON PAGE";
    printError(*RC_message);
    returnCode = RC_WRITE_FAILED;
  }
  else
  {
    returnCode = RC_OK;
  }

  // free memory to avoid memory leakage
  free(newblankPage);

  // Closing the file
  int cFile = fclose(fp);

  if (cFile == EOF)
  {
    // Handle file closure failure.
    RC_message = "ERR: FAILED TO CLOSE FILE";
    printError(*RC_message);
    returnCode = RC_CLOSE_FILE_FAILED;
  }

  return returnCode;
}

extern RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{

  RC returnCode = RC_OK;
  // Open Page for read and write purpose
  fp = fopen(fileName, "r+");

  if (!fp)
  {
    RC_message = "ERR: FILE NOT FOUND";
    printError(*RC_message);
    return returnCode = RC_FILE_NOT_FOUND;
  }

  // Initialize fields of fHandle
  fHandle->fileName = fileName;
  fHandle->totalNumPages = 0;
  fHandle->curPagePos = 0;
  fHandle->mgmtInfo = fp;

  // Read the total number of pages from the file (if available)
  fseek(fp, 0L, SEEK_END);
  fHandle->totalNumPages = ftell(fp) / PAGE_SIZE;

  // Return success code
  return returnCode;
}

extern RC closePageFile(SM_FileHandle *fHandle)
{
  // Check if the file handle is NULL or if the file is not initialized
  fp = fopen(fHandle->fileName, "r+");
  printf("************CLOSING PAGE FILE****************\n");
  int returnCode = RC_OK;
  if (fHandle == NULL || !fp)
  {
    returnCode = RC_FILE_NOT_FOUND;
    RC_message = "ERR: FILE DOES NOT EXIST!";
    printError(*RC_message);
    return returnCode;
  }

  // Close the file
  int closeStatus = fclose(fHandle->mgmtInfo);

  if (closeStatus == 0)
  {
    returnCode = RC_OK;
    RC_message = "CLOSED SUCCESSFULLY";
    printError(*RC_message);
    return returnCode;
  }
  else
  {
    returnCode = RC_CLOSE_FILE_FAILED;
    RC_message = "ERR: UNABLE TO CLOSE FILE";
    printError(*RC_message);
    return returnCode;
  }
}

extern RC destroyPageFile(char *fileName)
{
  int returnCode = RC_OK;
  // Close the file if it is Open
  if (fp != NULL)
  {
    fclose(fp);
    fp = NULL;
  }
  // Delete the File
  int destroyPageFileStatus = remove(fileName);

  // Check if success, return Appropriate Return codes
  if (destroyPageFileStatus == 0)
  {
    returnCode = RC_OK;
    RC_message = "DESTROYED SUCCESFULLY";
    printError(*RC_message);
    return returnCode;
  }
  else
  {
    returnCode = RC_FILE_NOT_FOUND;
    RC_message = "ERR: FAILED TO DESTROY PAGE FILE!!";
    printError(*RC_message);
    return returnCode;
  }
}

// Author: Rana Feyza Soylu
extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  if (fHandle == NULL)
  { // Making sure file has been initialized/opened
    return RC_FILE_HANDLE_NOT_INIT;
  }
  if (fHandle->totalNumPages - 1 < pageNum || pageNum < 0)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }
  else
  {
    // Placing the pointer at the start of the block that we want to read
    int rval = fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE, SEEK_SET); // SEEK_SET starts it from the beginning of the file, the offset is the total amount we need to move to get to pageNum
    if (rval == 0)
    {
      fHandle->curPagePos = pageNum;                   // Setting current page position to pagenum
      fread(memPage, 1, PAGE_SIZE, fHandle->mgmtInfo); // Read one page
      return RC_OK;
    }
    else
    {
      return RC_MEM_ALLOC_FAILED;
    }
  }
}

// Author: Rana Feyza Soylu
extern int getBlockPos(SM_FileHandle *fHandle)
{
  if (fHandle == NULL)
  { // Making sure file has been initialized/opened
    return RC_FILE_HANDLE_NOT_INIT;
  }
  return fHandle->curPagePos; // Returns the current page/block position
}

// Author: Rana Feyza Soylu
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  if (fHandle == NULL)
  { // Making sure file has been initialized/opened
    return RC_FILE_HANDLE_NOT_INIT;
  }
  return readBlock(0, fHandle, memPage); // Reads first block using predefined function
}

extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // Checking if the file handle is not initialized
  if (fHandle == NULL)
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }
  // Retrieve the current position in the file
  int cP = fHandle->curPagePos;
  // If the current position is at the start, there's no previous page to fetch
  if (cP == 0)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // Rreading the previous page based on the current position
  return readBlock(cP - 1, fHandle, memPage);
}

extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // Validate the file handle initialization
  if (fHandle == NULL)
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }
  // Obtain the current page position
  int cP = fHandle->curPagePos;
  // Checking if the current position is valid within the file's range
  if (cP < 0 || cP >= fHandle->totalNumPages)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // Fetching the current page using the current position

  return readBlock(cP, fHandle, memPage);
}

extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // Ensure the file handle has been properly initialized
  if (fHandle == NULL)
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }
  // Determine the current page's position
  int cP = fHandle->curPagePos;
  // If the current position is at the last page, there is no next page
  if (cP >= fHandle->totalNumPages - 1)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // Reading the next page by incrementing the current position
  return readBlock(cP + 1, fHandle, memPage);
}

extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // Check for an uninitialized file handle
  if (fHandle == NULL)
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }
  // Get the position of the last page in the file
  int fP = fHandle->totalNumPages - 1;
  // If there are no pages in the file, return an error
  if (fP < 0)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }

  // Accessing the last page
  return readBlock(fP, fHandle, memPage);
}

/* writing blocks to a page file */

RC seekToPage(SM_FileHandle *fHandle, int pageNum)
{
  int offset = pageNum * PAGE_SIZE * sizeof(char);
  if (fseek(fHandle->mgmtInfo, offset, SEEK_SET) != 0)
  {
    return RC_SEEK_ERROR;
  }
  return RC_OK;
}
int seekToEnd(FILE *handle, long long fSet, int seekLoc)
{
  return fseek(handle, fSet, seekLoc);
}

RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{

  if (fHandle->mgmtInfo == NULL)
  {
    return RC_WRITE_FAILED;
  }
  else if (pageNum < 0 || pageNum >= fHandle->totalNumPages)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }

  RC rc = seekToPage(fHandle, pageNum);
  if (rc != RC_OK)
  {
    return rc;
  }

  size_t num_items = fwrite(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);
  if (num_items != PAGE_SIZE)
  {
    return RC_WRITE_FAILED;
  }

  fHandle->curPagePos = pageNum;
  return RC_OK;
}

// Function to write data from a memory page to the current block of a file
RC writeCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memoryPage)
{
  // Get the block position of the current page in the file
  int blockPos = getBlockPos(fileHandle);

  // Write the block data to the file using the memory page
  RC rc = writeBlock(blockPos, fileHandle, memoryPage);

  // Return the result code from the writeBlock function
  return rc;
}

extern RC appendEmptyBlock(SM_FileHandle *fHandle)
{
  int seekResult;
  size_t writeBlockSize;
  SM_PageHandle emptyPage;

  // Allocate memory for an empty page
  emptyPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
  if (emptyPage == NULL)
  {
    return RC_WRITE_FAILED;
  }

  // Set pointer to the end of the file
  seekResult = seekToEnd(fHandle->mgmtInfo, 0, SEEK_END);
  if (seekResult != 0)
  {
    free(emptyPage);
    return RC_WRITE_FAILED;
  }

  // Write the empty page to the file
  writeBlockSize = fwrite(emptyPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);
  if (writeBlockSize != PAGE_SIZE)
  {
    free(emptyPage);
    return RC_WRITE_FAILED;
  }

  // Update file handle information
  fHandle->totalNumPages += 1;
  fHandle->curPagePos = fHandle->totalNumPages;

  // Rewind and update total number of pages in the file
  rewind(fHandle->mgmtInfo);
  fprintf(fHandle->mgmtInfo, "%d\n", fHandle->totalNumPages);

  // Set pointer to the beginning of the file
  seekToEnd(fHandle->mgmtInfo, 0, SEEK_SET);

  // Clean up allocated memory
  free(emptyPage);

  return RC_OK;
}

/*
 * Ensures that the given file handle has a capacity of at least the specified number of pages.
 * If the current capacity is less than the desired capacity, empty blocks will be appended to the file
 * until the desired capacity is reached.
 *
 * Parameters:
 *  numberOfPages - The desired capacity of the file handle, in pages.
 *  fHandle - The file handle to ensure capacity for.
 *
 * Returns:
 *  RC_OK if the capacity was successfully ensured, or RC_ALLOCATION_FAILED if the allocation of new blocks failed.
 */
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{

  /* While the current capacity is less than the desired capacity*/

  while (fHandle->totalNumPages < numberOfPages)
  {

    // Attempting to append an empty block to the file handle.

    RC rc = appendEmptyBlock(fHandle);

    // If the allocation of the new block failed, return an error code.
    if (rc != RC_OK)
    {

      /*Returning RC_ALLOCATION_FAILED*/
      return RC_ALLOCATION_FAILED;
    }
  }

  // If the loop completed without returning an error, the capacity was successfully ensured.
  return RC_OK;
}
