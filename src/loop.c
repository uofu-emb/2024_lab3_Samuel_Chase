#include "loop.h"

#define SLEEPTIME 5  // Define a constant for sleep time in some future use

// Function to perform an action in a loop with semaphore synchronization
int do_loop(SemaphoreHandle_t semaphore,
            int *counter,
            const char *src,
            TickType_t timeout)
{
    // Attempt to take the semaphore within the given timeout
    if (xSemaphoreTake(semaphore, timeout) == pdFALSE)
        return pdFALSE;  // Return false if semaphore can't be taken

    {
        // Increment the shared counter and print a message indicating the source
        (*counter)++;
        printf("hello world from %s! Count %d\n", src, *counter);
    }

    // Release the semaphore after completing the task
    xSemaphoreGive(semaphore);
    return pdTRUE;  // Return true to indicate success
}

// Task that intentionally creates a deadlock scenario
void deadlock(void *args)
{
    struct DeadlockArgs *dargs = (struct DeadlockArgs *)args;  // Cast the args to the proper struct type

    printf("\tinside deadlock %c\n", dargs->id);
    dargs->counter++;  // Increment the counter before attempting to lock

    // Take the first semaphore (a) and proceed
    xSemaphoreTake(dargs->a, portMAX_DELAY);
    {
        dargs->counter++;  // Increment the counter after acquiring the first semaphore
        printf("\tinside first lock %c\n", dargs->id);

        // Simulate some work with a delay
        vTaskDelay(100);
        printf("\tpost-delay %c\n", dargs->id);

        // Attempt to take the second semaphore (b), potentially causing deadlock
        xSemaphoreTake(dargs->b, portMAX_DELAY);
        {
            printf("\tinside second lock %c\n", dargs->id);
            dargs->counter++;  // Increment the counter after acquiring the second semaphore
        }

        // Release the second semaphore (b)
        xSemaphoreGive(dargs->b);
    }

    // Release the first semaphore (a)
    xSemaphoreGive(dargs->a);

    // Suspend the task, simulating the deadlock completion
    vTaskSuspend(NULL);
}

// Function that attempts to lock a semaphore and increments a counter
// May 'orphan' the semaphore if it returns without giving it back
int orphaned_lock(SemaphoreHandle_t semaphore, TickType_t timeout, int *counter)
{
    // Attempt to take the semaphore within the given timeout
    if (xSemaphoreTake(semaphore, timeout) == pdFALSE)
        return pdFALSE;  // Return false if the semaphore can't be taken

    {
        // Increment the counter
        (*counter)++;

        // If the counter is odd, return without giving back the semaphore (orphaned lock)
        if (*counter % 2) {
            return 0;
        }

        // Otherwise, print the count and release the semaphore
        printf("Count %d\n", *counter);
    }

    // Release the semaphore if the counter is even
    xSemaphoreGive(semaphore);
    return pdTRUE;  // Return true to indicate success
}

// Function that ensures the semaphore is not orphaned
int unorphaned_lock(SemaphoreHandle_t semaphore, TickType_t timeout, int *counter)
{
    // Attempt to take the semaphore within the given timeout
    if (xSemaphoreTake(semaphore, timeout) == pdFALSE)
        return pdFALSE;  // Return false if the semaphore can't be taken

    {
        // Increment the counter
        (*counter)++;

        // If the counter is even, print the count
        if (!(*counter % 2)) {
            printf("Count %d\n", *counter);
        }
    }

    // Always release the semaphore to avoid orphaning it
    xSemaphoreGive(semaphore);
    return pdTRUE;  // Return true to indicate success
}
