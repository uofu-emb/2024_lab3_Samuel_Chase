#pragma once

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>

struct DeadlockArgs {
    SemaphoreHandle_t a;
    SemaphoreHandle_t b;
    int counter;
    char id;
};
void deadlock(void *);

int do_loop(SemaphoreHandle_t semaphore,
            int *counter,
            const char *src,
            TickType_t timeout);

int unorphaned_lock(SemaphoreHandle_t semaphore, TickType_t timeout, int *counter);
int orphaned_lock(SemaphoreHandle_t semaphore, TickType_t timeout, int *counter);