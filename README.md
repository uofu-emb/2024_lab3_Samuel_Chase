## Current Workflow Status

![example workflow](https://github.com/uofu-emb/2024_lab3_Samuel_Chase/actions/workflows/main.yml/badge.svg)

# Lab 3 Threads (Sam Bosch, Chase Griswold)
Full disclosure: Relied heavily on reference implemention, however, we extensively commented the code and documented the architecture and functionality in this README.

# Objectives
Setup the operating system.
Setup multiple threads.
Identify shared state and race conditions.
Protect critical sections.
Write unit tests.


# Thread Status Screenshot

# Thread Testing Screenshot
![Application Screenshot](https://github.com/uofu-emb/2024_lab3_Samuel_Chase/blob/chase_branch1/test_screenshot.png)

# Overview of the Code

This document provides a detailed explanation of the provided code files, which implement semaphore-based synchronization, deadlock simulation, and testing in an embedded FreeRTOS environment using the Pico SDK.

---

## `loop.h` and `loop.c`

### Functionality

1. **`do_loop` Function**  
   Synchronizes a loop using a semaphore to increment a shared counter and print messages. 
   - **Input Parameters:**
     - `SemaphoreHandle_t semaphore`: Semaphore to ensure exclusive access.
     - `int *counter`: Pointer to the shared counter.
     - `const char *src`: Source identifier string.
     - `TickType_t timeout`: Maximum time to wait for semaphore acquisition.
   - **Behavior:** 
     - Acquires semaphore or returns `pdFALSE` if timed out.
     - Increments the counter and prints the current count.
     - Releases the semaphore after execution.

2. **`deadlock` Task**  
   Simulates a deadlock scenario by acquiring two semaphores in a nested manner.  
   - **Inputs:**  
     - Struct containing two semaphores (`a` and `b`), a counter, and an identifier (`id`).  
   - **Behavior:**  
     - Acquires one semaphore, waits, and attempts to acquire another, potentially causing a deadlock.

3. **`orphaned_lock` Function**  
   Deliberately creates a scenario where a semaphore may not be released.  
   - **Input Parameters:**  
     - `SemaphoreHandle_t semaphore`: Semaphore to lock.  
     - `TickType_t timeout`: Timeout for semaphore acquisition.  
     - `int *counter`: Shared counter.  
   - **Behavior:**  
     - Increments the counter but releases the semaphore only if the counter is even.

4. **`unorphaned_lock` Function**  
   Ensures a semaphore is always released, avoiding orphaned locks.  
   - **Input Parameters:** Same as `orphaned_lock`.  
   - **Behavior:**  
     - Increments the counter and always releases the semaphore.

---

## Main Application Code ('threads.c`)

### Features

1. **Thread Synchronization with Semaphore**  
   - A counting semaphore is used to synchronize a shared counter between two tasks.

2. **Tasks**  
   - **`main_thread`:**  
     - Toggles an onboard LED and increments the counter in a loop using the semaphore.  
   - **`side_thread`:**  
     - Periodically increments the shared counter in parallel with `main_thread`.  

3. **Initialization and Scheduler Start**  
   - Initializes the semaphore, tasks, and scheduler.  
   - The `main_thread` and `side_thread` are created with equal priorities.

---

## Test Code (`test.c`)

### Testing Framework

- **Unity Framework**: Used for unit testing.

### Test Cases

1. **Activity 0-2: `do_loop` Tests**
   - **`test_loop_blocks`**: Verifies that `do_loop` blocks when the semaphore is unavailable.  
   - **`test_loop_runs`**: Verifies that `do_loop` executes correctly when the semaphore is available.

2. **Activity 3: Deadlock Simulation**
   - **`test_deadlock`**:  
     - Creates two tasks simulating a deadlock using two semaphores.  
     - Validates that both semaphores are locked and counters reflect deadlock behavior.

3. **Activity 4: Orphaned and Unorphaned Locks**
   - **`test_orphaned`**:  
     - Ensures that an orphaned lock behaves as expected by not releasing the semaphore in certain conditions.  
   - **`test_unorphaned`**:  
     - Verifies proper release of the semaphore in all cases.

---

## Main Test Runner (`test.c` Main Function)

- Initializes standard I/O and the Wi-Fi chip.  
- Creates a high-priority `runner_thread` to execute all test cases sequentially.  
- Uses Unity's `UNITY_BEGIN` and `UNITY_END` for test execution.

---

## Key Definitions

### Macros
- **Task Priorities:**
  - `MAIN_TASK_PRIORITY`, `SIDE_TASK_PRIORITY`, and `TEST_RUNNER_PRIORITY` define task execution priorities.
- **Task Stack Sizes:**
  - `MAIN_TASK_STACK_SIZE`, `SIDE_TASK_STACK_SIZE`, etc., determine stack memory allocation.

---

## Code Structure

### Header File
- **`loop.h`**: Defines function prototypes and structures for synchronization.

### Source Files
- **`loop.c`**: Implements semaphore operations and deadlock simulation.
- **`main.c`**: Implements task creation, semaphore usage, and scheduler initialization.
- **`test.c`**: Implements unit tests using Unity and verifies semaphore behavior under various conditions.

---

## Usage

1. Compile and flash the code onto the target board using the Pico SDK.  
2. Observe task and semaphore behavior through serial output.  
3. Run the tests to validate functionality and handle edge cases.

