#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "loop.h"
#include "hardware/adc.h"

#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define MAIN_TASK_STACK_SIZE    configMINIMAL_STACK_SIZE
#define SIDE_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define SIDE_TASK_STACK_SIZE    configMINIMAL_STACK_SIZE

SemaphoreHandle_t semaphore;
int counter;
int on;

float read_temperature() {
    adc_init();                      // Initialize ADC
    adc_select_input(4);              // Select input 4 for temperature sensor
    uint16_t raw = adc_read();        // Read raw ADC value
    const float conversion_factor = 3.3f / (1 << 12); // Conversion factor for 12-bit ADC
    float voltage = raw * conversion_factor;  // Calculate voltage
    return 27.0f - (voltage - 0.706f) / 0.001721f;  // Formula from RP2040 datasheet
}

void side_thread(void *params)
{
    while (1) {
        vTaskDelay(100);
        float temp = read_temperature();
        printf("Side Thread: Temperature = %.2f °C\n", temp);
        do_loop(semaphore, &counter, "Side Thread", 500);
    }
}

void main_thread(void *params)
{
    while (1) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
        vTaskDelay(100);
        printf("Main Thread: Performing LED toggle\n");
        do_loop(semaphore, &counter, "Main Thread", 500);
        on = !on;
    }
}

int main(void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);

    on = false;
    counter = 0;

    TaskHandle_t main, side;
    semaphore = xSemaphoreCreateCounting(1, 1);

    xTaskCreate(main_thread, "MainThread", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &main);
    xTaskCreate(side_thread, "SideThread", SIDE_TASK_STACK_SIZE, NULL, SIDE_TASK_PRIORITY, &side);

    vTaskStartScheduler();

    return 0;
}

// #include <stdio.h>
// #include <FreeRTOS.h>
// #include <semphr.h>
// #include <task.h>
// #include <pico/stdlib.h>
// #include <pico/multicore.h>
// #include <pico/cyw43_arch.h>
// #include "loop.h"  // Include the custom loop.h header for the do_loop function

// // Define the priority and stack size for the main task
// #define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
// #define MAIN_TASK_STACK_SIZE    configMINIMAL_STACK_SIZE

// // Define the priority and stack size for the side task
// #define SIDE_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
// #define SIDE_TASK_STACK_SIZE    configMINIMAL_STACK_SIZE

// // Semaphore to control access to shared resources
// SemaphoreHandle_t semaphore;

// // Shared counter variable used by both threads
// int counter;
// // Variable to control the LED state
// int on;

// // Function that runs in the side thread (task)
// void side_thread(void *params)
// {
//     while (1) {
//         vTaskDelay(100);  // Delay for 100 ticks (acts like a sleep)
//         // Call the do_loop function with the shared semaphore, counter, and "side" identifier
//         do_loop(semaphore, &counter, "Side Thread", 500);  // Process the loop for the side thread
//     }
// }

// // Function that runs in the main thread (task)
// void main_thread(void *params)
// {
//     while (1) {
//         // Control the onboard LED based on the value of 'on'
//         cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
        
//         vTaskDelay(100);  // Delay for 100 ticks
//         // Call the do_loop function with the shared semaphore, counter, and "main" identifier
//         do_loop(semaphore, &counter, "Main Thread", 500);  // Process the loop for the main thread

//         // Toggle the 'on' variable (LED state)
//         on = !on;
//     }
// }

// // Main function where execution starts
// int main(void)
// {
//     // Initialize standard I/O for printing messages
//     stdio_init_all();

//     // Initialize the Wi-Fi chip (CYW43) and ensure it succeeds
//     hard_assert(cyw43_arch_init() == PICO_OK);

//     // Initialize variables
//     on = false;   // Start with the LED off
//     counter = 0;  // Initialize the counter to 0

//     // Create task handles for the main and side threads
//     TaskHandle_t main, side;

//     // Create a counting semaphore to control access to the shared counter
//     semaphore = xSemaphoreCreateCounting(1, 1);

//     // Create the main thread with the specified priority and stack size
//     xTaskCreate(main_thread, "MainThread",
//                 MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &main);

//     // Create the side thread with the specified priority and stack size
//     xTaskCreate(side_thread, "SideThread",
//                 SIDE_TASK_STACK_SIZE, NULL, SIDE_TASK_PRIORITY, &side);

//     // Start the FreeRTOS task scheduler, which begins executing the tasks
//     vTaskStartScheduler();

//     return 0;  // This line is never reached as the scheduler takes over
// }
