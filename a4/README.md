# Virtual Memory Page Replacement Simulator

## Overview

This program simulates how an operating system handles **virtual memory page replacement** using three different policies: **FIFO**, **LRU**, and **CLOCK**. It translates logical memory addresses into physical ones, managing page faults and memory frames in the process.

## Purpose

This project was developed as part of an Operating Systems course to gain a deeper understanding of how low-level memory management works. The goal was to implement page replacement strategies and analyze their performance on realistic memory traces, highlighting the tradeoffs between policies.

## Implementation

The simulation is written in **C** and builds on a provided skeleton file (`virtmem.c`) that handles command-line parsing, address translation, and trace file reading. I extended this base by implementing logic for:

- Maintaining and updating a page table
- Handling page faults
- Simulating FIFO, LRU, and CLOCK page replacement
- Tracking swap-ins and swap-outs

Each replacement algorithm maintains its own internal data structure (e.g., queue, timestamp array, or circular buffer), integrated into the `resolve_address()` function.

## Input

The input is the name of a **memory trace file** containing a list of memory accesses (instruction fetch, memory read, memory write). The file will follow this format:

```
I: 0x7feee195f090
W: 0x7ffe23dd2e88
R: 0x7ffe23dd2e78
```

The trace files below are located in this directory.
- `hello-out.txt`
- `ls-out.txt`
- `matrixmult-out.txt`

## Output

The simulator prints a summary report after processing all memory references, including:
- Total number of memory references
- Number of page faults
- Swap-ins
- Swap-outs

## Example (1) 

Input:
./virtmem --file= hello-out.txt --framesize=12 --numframes=256 --replace=fifo --progress

Output:
Progress [............................................................] 100%
Memory references: 127926
Page faults: 119
Swap ins: 119
Swap outs: 0


## Example (2) 

Input:
./virtmem --file=hello-out.txt --framesize=12     --numframes=100 --replace=*POLICY* --progress

Output (FIFO):
Progress [............................................................] 100%
Memory references: 127926
Page faults: 131
Swap ins: 131
Swap outs: 7

Output (LRU):
Progress [............................................................] 100%
Memory references: 127926
Page faults: 119
Swap ins: 119
Swap outs: 1

Output (CLOCK):
Progress [............................................................] 100%
Memory references: 127926
Page faults: 123
Swap ins: 123
Swap outs: 3

## How to Run

### 1. Compile the Program

Run:
```bash
make
```

This uses the provided `Makefile` to compile `virtmem.c` into an executable named `virtmem`.

### 2. Run the Simulator

Example:
```bash
./virtmem --file=hello-out.txt --framesize=12 --numframes=256 --replace=lru --progress
```

### Command-Line Arguments

| Argument            | Description |
|---------------------|-------------|
| `--file=<path>`     | Path to the trace file (e.g. `traces/hello-out.txt`) |
| `--framesize=<n>`   | Frame size as 2^n bytes (e.g. 12 → 4096 bytes) |
| `--numframes=<n>`   | Total number of physical frames |
| `--replace=<algo>`  | Page replacement algorithm: `fifo`, `lru`, or `clock` |
| `--progress`        | Shows a progress bar during simulation |

---

## Credits

Skeleton code and assignment design by Michael Zastre.  
Modified for UVic CSC360, Spring 2025, by Konrad Jasman. 
