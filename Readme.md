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
