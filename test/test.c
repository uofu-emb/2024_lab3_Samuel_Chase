#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include <unity.h>  // Include Unity for unit testing framework
#include "loop.h"   // Custom loop functions and definitions

// The test runner task has the highest priority.
// In FreeRTOS, a higher number indicates a higher priority.
#define TEST_RUNNER_PRIORITY ( tskIDLE_PRIORITY + 5UL )

// Required by Unity framework, called before each test
void setUp(void) {}

// Required by Unity framework, called after each test
void tearDown(void) {}

/**************** Activity 0-2 ****************/
// Test case to check if the loop function blocks as expected
void test_loop_blocks(void)
{
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1, 1);
    int counter = 0;
    xSemaphoreTake(semaphore, portMAX_DELAY);  // Take semaphore so it's unavailable

    // Call the loop and check if it blocks (returns pdFALSE)
    int result = do_loop(semaphore, &counter, "test", 10);

    // Test that loop blocked and the counter did not increment
    TEST_ASSERT_EQUAL_INT(pdFALSE, result);
    TEST_ASSERT_EQUAL_INT(0, counter);
}

// Test case to check if the loop runs correctly when semaphore is available
void test_loop_runs(void)
{
    int counter = 0;
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1, 1);
    
    // Call the loop and check if it runs (returns pdTRUE)
    int result = do_loop(semaphore, &counter, "test", 10);

    // Test that loop ran and the counter incremented
    TEST_ASSERT_EQUAL_INT(pdTRUE, result);
    TEST_ASSERT_EQUAL_INT(1, counter);
}

/**************** Activity 3 ****************/
// Define task stack sizes and priorities for deadlock test
#define LEFT_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define LEFT_TASK_PRIORITY   ( TEST_RUNNER_PRIORITY - 1UL )
#define RIGHT_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define RIGHT_TASK_PRIORITY  ( TEST_RUNNER_PRIORITY - 1UL )

// Test case for detecting deadlock between two tasks
void test_deadlock(void)
{
    TaskHandle_t left_thread, right_thread;
    SemaphoreHandle_t left = xSemaphoreCreateCounting(1, 1);  // Semaphore for left lock
    SemaphoreHandle_t right = xSemaphoreCreateCounting(1, 1); // Semaphore for right lock

    printf("- Creating threads\n");

    // Define arguments for deadlock tasks
    struct DeadlockArgs left_args = {left, right, 0, 'a'};
    struct DeadlockArgs right_args = {right, left, 7, 'b'};

    // Create tasks for left and right threads simulating deadlock
    BaseType_t left_status = xTaskCreate(deadlock, "Left", LEFT_TASK_STACK_SIZE,
                                         (void *)&left_args, LEFT_TASK_PRIORITY, &left_thread);
    BaseType_t right_status = xTaskCreate(deadlock, "Right", RIGHT_TASK_STACK_SIZE,
                                          (void *)&right_args, RIGHT_TASK_PRIORITY, &right_thread);

    printf("- Created threads\n");

    // Wait for 1000 ticks before checking for deadlock
    vTaskDelay(1000);
    printf("- Waited 1000 ticks\n");

    // Check semaphore counts and task states
    TEST_ASSERT_EQUAL_INT(uxSemaphoreGetCount(left), 0);  // Left semaphore should be locked
    TEST_ASSERT_EQUAL_INT(uxSemaphoreGetCount(right), 0); // Right semaphore should be locked
    TEST_ASSERT_EQUAL_INT(2, left_args.counter);  // Left task should have incremented its counter twice
    TEST_ASSERT_EQUAL_INT(9, right_args.counter); // Right task should have incremented its counter twice

    printf("- Killing threads\n");
    vTaskDelete(left_thread);  // Clean up left task
    vTaskDelete(right_thread); // Clean up right task
    printf("- Killed threads\n");
}

/**************** Activity 4 ****************/
// Test case to check if an orphaned lock behaves correctly
void test_orphaned(void)
{
    int counter = 1;
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1, 1);

    // First call to orphaned_lock: should succeed and increment the counter
    int result = orphaned_lock(semaphore, 500, &counter);
    TEST_ASSERT_EQUAL_INT(2, counter);
    TEST_ASSERT_EQUAL_INT(pdTRUE, result);
    TEST_ASSERT_EQUAL_INT(1, uxSemaphoreGetCount(semaphore)); // Semaphore should be released

    // Second call: should fail because counter is now odd
    result = orphaned_lock(semaphore, 500, &counter);
    TEST_ASSERT_EQUAL_INT(3, counter);
    TEST_ASSERT_EQUAL_INT(pdFALSE, result);
    TEST_ASSERT_EQUAL_INT(0, uxSemaphoreGetCount(semaphore)); // Semaphore is still locked

    // Third call: no further increment, same conditions as previous
    result = orphaned_lock(semaphore, 500, &counter);
    TEST_ASSERT_EQUAL_INT(3, counter);
    TEST_ASSERT_EQUAL_INT(pdFALSE, result);
    TEST_ASSERT_EQUAL_INT(0, uxSemaphoreGetCount(semaphore)); // Semaphore remains locked
}

// Test case to check behavior when lock is not orphaned
void test_unorphaned(void)
{
    int counter = 1;
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1, 1);

    // First call to unorphaned_lock: should succeed and increment counter to 2
    int result = unorphaned_lock(semaphore, 500, &counter);
    TEST_ASSERT_EQUAL_INT(2, counter);
    TEST_ASSERT_EQUAL_INT(pdTRUE, result);
    TEST_ASSERT_EQUAL_INT(1, uxSemaphoreGetCount(semaphore)); // Semaphore should be released

    // Second call: should succeed again and increment counter to 3
    result = unorphaned_lock(semaphore, 500, &counter);
    TEST_ASSERT_EQUAL_INT(3, counter);
    TEST_ASSERT_EQUAL_INT(pdTRUE, result);
    TEST_ASSERT_EQUAL_INT(1, uxSemaphoreGetCount(semaphore)); // Semaphore released again
}

/**************** runner ****************/
// The main runner thread that executes all tests
void runner_thread(__unused void *args)
{
    for (;;) {
        printf("Start tests\n");
        UNITY_BEGIN();  // Start Unity test framework
        RUN_TEST(test_loop_blocks);  // Run individual tests
        RUN_TEST(test_loop_runs);
        RUN_TEST(test_deadlock);
        RUN_TEST(test_orphaned);
        RUN_TEST(test_unorphaned);

        UNITY_END();  // End Unity test framework
        sleep_ms(10000);  // Pause for 10 seconds before running tests again
    }
}

int main(void)
{
    stdio_init_all();  // Initialize standard I/O for printing
    hard_assert(cyw43_arch_init() == PICO_OK);  // Initialize Wi-Fi chip (CYW43) and check success

    // Create the test runner thread
    xTaskCreate(runner_thread, "TestRunner", configMINIMAL_STACK_SIZE, NULL, TEST_RUNNER_PRIORITY, NULL);

    // Start the FreeRTOS scheduler to begin task execution
    vTaskStartScheduler();
    
    return 0;  // This line will never be reached since the scheduler is running
}
