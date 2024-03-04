# CS 525 Buffer Manager

    Contributors:
    - Abhijnan Acharya A20536263
    - Tanushree Halder A20554883
    - Roshan Hyalij A20547441
    - Rana Feyza Soylu A20465152

---

---

**function BM initBufferPool**

This file describes the `initBufferPool` function for initializing a Buffer Manager buffer pool.

**Function Signature:**

```c
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData);
```

**Purpose:** Initializes a `BM_BufferPool` struct for managing a pool of memory pages for disk I/O buffering.

**Parameters:**

- `bm`: Pointer to the `BM_BufferPool` struct to be initialized.
- `pageFileName`: Name of the file.
- `numPages`: Number of pages in the buffer pool.
- `strategy`: Replacement strategy to use (e.g., `FIFO`, `LRU`).
- `stratData`: Strategy-specific data.

**Return:**

- `RC_OK`: Initialization successful.

**Details:**

1. Checks for null `bm` and initializes properties.
2. Allocates memory for the frame array.
3. Initializes each frame.
4. Sets management data and returns status.

**function shutDownBufferPool**
This file describes the 'shutDownBufferPool' function for shutting down a buffer pool.

**Function Signature:**

```c
extern RC shutdownBufferPool(BM_BufferPool *const bm);
```

**Purpose:** Destroys the buffer pool and deallocates all the resources associated with the buffer pool. 

**Parameters:**

- `bm`: Pointer to the `BM_BufferPool` struct to be initialized.

**Return:**

- `RC_OK`: Initialization successful.

**Details:**

1. Initializes pointer to a frame and assigns it the value of bm->mgmtData
2. Calls a function named forceFlushPool passing the bm pointer as an argument.
3. Checks if the fix count of the current page frame is not equal to zero. If any page frame has a non-zero fix count, it means that some client is currently using that page, so the function returns RC_PAGES_IN_BUFFER.*/
4. Sets the mgmtData field of the buffer pool structure pointed to by bm to NULL. This indicates that the buffer pool is now empty.
5. Returns status.


**function flushFrame**
This file describes the 'shutDownBufferPool' function for shutting down a buffer pool.

**Function Signature:**

```c
(BM_BufferPool *const bm, Frame *pageFrame, int pageNum);
```

**Purpose:** Causes all dirty pages (with fix count 0) from the buffer pool to be written to disk. 

**Parameters:**

- `bm`: Pointer to the `BM_BufferPool` struct to be initialized.
- `pageFrame`: Data from the given frame.
- `pageNum`: The number of the page frame to be flushed.

**Return:**

- `void`: Nothing is returned.

**Details:**

1. Opens the page file linked to the buffer pool.
2. Using Writeblock, the data from the page frame is written to the matching page number.
3. The frame is designated as clean by changing the dirtyCount field to 0.
4. Modifies a counter to keep track of how many pages have been written to disk.



