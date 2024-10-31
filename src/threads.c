#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "loop.h"  // Include the custom loop.h header for the do_loop function

// Define the priority and stack size for the main task
#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define MAIN_TASK_STACK_SIZE    configMINIMAL_STACK_SIZE

// Define the priority and stack size for the side task
#define SIDE_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define SIDE_TASK_STACK_SIZE    configMINIMAL_STACK_SIZE

// Semaphore to control access to shared resources
SemaphoreHandle_t semaphore;

// Shared counter variable used by both threads
int counter;
// Variable to control the LED state
int on;

// Function that runs in the side thread (task)
void side_thread(void *params)
{
    while (1) {
        vTaskDelay(100);  // Delay for 100 ticks (acts like a sleep)
        // Call the do_loop function with the shared semaphore, counter, and "side" identifier
        do_loop(semaphore, &counter, "side", 500);  // Process the loop for the side thread
    }
}

// Function that runs in the main thread (task)
void main_thread(void *params)
{
    while (1) {
        // Control the onboard LED based on the value of 'on'
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
        
        vTaskDelay(100);  // Delay for 100 ticks
        // Call the do_loop function with the shared semaphore, counter, and "main" identifier
        do_loop(semaphore, &counter, "main", 500);  // Process the loop for the main thread

        // Toggle the 'on' variable (LED state)
        on = !on;
    }
}

// Main function where execution starts
int main(void)
{
    // Initialize standard I/O for printing messages
    stdio_init_all();

    // Initialize the Wi-Fi chip (CYW43) and ensure it succeeds
    hard_assert(cyw43_arch_init() == PICO_OK);

    // Initialize variables
    on = false;   // Start with the LED off
    counter = 0;  // Initialize the counter to 0

    // Create task handles for the main and side threads
    TaskHandle_t main, side;

    // Create a counting semaphore to control access to the shared counter
    semaphore = xSemaphoreCreateCounting(1, 1);

    // Create the main thread with the specified priority and stack size
    xTaskCreate(main_thread, "MainThread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &main);

    // Create the side thread with the specified priority and stack size
    xTaskCreate(side_thread, "SideThread",
                SIDE_TASK_STACK_SIZE, NULL, SIDE_TASK_PRIORITY, &side);

    // Start the FreeRTOS task scheduler, which begins executing the tasks
    vTaskStartScheduler();

    return 0;  // This line is never reached as the scheduler takes over
}
