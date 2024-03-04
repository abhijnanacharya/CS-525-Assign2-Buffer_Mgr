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


---

---

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

---

---

**function flushFrame**

This file describes the 'flushFrame' function which flushes a frame.

**Function Signature:**

```c
void flushFrame(BM_BufferPool *const bm, Frame *pageFrame, int pageNum);
```

**Purpose:** Flushes one page frame to the disk.

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



---

---

**function forceFlushPool**

This file describes the 'flushFrame' function which flushes each frame in the buffer pool using the flushFrame function.

**Function Signature:**

```c
extern RC forceFlushPool(BM_BufferPool *const bm);
```

**Purpose:** Causes all dirty pages (with fix count 0) from the buffer pool to be written to disk.


**Parameters:**

- `bm`: Pointer to the `BM_BufferPool` struct to be initialized.

**Return:**

- `RC_OK`: Initialization successful.

**Details:**

1. Iterates over each frame in the buffer pool.
2. For each frame, if it's not currently in use and has been modified, it calls flushFrame to write its data to disk.
3. The function returns RC_OK, ensuring all dirty pages are successfully flushed to maintain data consistency.



---

---

**function markDirty**

This file describes the 'markDirty' function which marks a page as dirty.

**Function Signature:**

```c
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page);
```

**Purpose:** This function marks a given page as dirty.


**Parameters:**

- `bm`: Pointer to the `BM_BufferPool` struct to be initialized.
- `page`: Given page to check if dirty or not.

**Return:**

- `RC_OK`: Initialization successful.
- `RC_READ_NON_EXISTING_PAGE`: Tried to read a non existent page error.

**Details:**

1. Checks if buffer manager, page file, or page handle pointers are NULL.
2. Marks the frame as dirty since the page has been modified
3. Returns either true or page not found error



---

---

**function unpinPage**

This file describes the 'unpinPage ' function which unpins a page.

**Function Signature:**

```c
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page);
```

**Purpose:** This function unpins a given page.


**Parameters:**

- `bm`: Pointer to the `BM_BufferPool` struct to be initialized.
- `page`: Given page to unpin.

**Return:**

- `RC_OK`: Initialization successful.
- `RC_READ_NON_EXISTING_PAGE`: Tried to read a non existent page error.




---

---

**function forcePage**

This file describes the 'forcePage ' function which writes the current content of the page back to the page file on disk.

**Function Signature:**

```c
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page);
```

**Purpose:** This function writes the current content of the page back to the page file on disk.


**Parameters:**

- `bm`: Pointer to the `BM_BufferPool` struct to be initialized.
- `page`: Given page to write back to disk.

**Return:**

- `RC_OK`: Operation successful.
- `RC_FILE_NOT_FOUND`: Tried to read a non existent page file error.

**Details:**
1. Attempts to open the page file.
2. Writes the page data to disk.
3. Closes the file handle after successful write.


