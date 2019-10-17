#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "Heap.h"
#include "MockHeapHelper.h"
#include "MockAssert.h"
#include "unity.h"

#define ARRAY_SIZE     10
#define MAX_ALLOC_SIZE 100
#define TRACE(...) // printf(__VA_ARGS__)

static void EnterCriticalSection(void);
static void ExitCriticalSection(void);

static const struct HeapConfig TC_HeapConfig =
{
    .corrupted_data_hook    = CorruptedDataHook,
    .alloc_failed_hook      = AllocFailedHook,
    .invalid_pointer_hook   = InvalidPointerHook,
    .enter_critical_section = EnterCriticalSection,
    .exit_critical_section  = ExitCriticalSection,
};

static bool  TC_IsCriticalSectionActive;
static void* TC_Array[10];

static void EnterCriticalSection(void)
{
    TEST_ASSERT_FALSE(TC_IsCriticalSectionActive);
    TC_IsCriticalSectionActive = true;
}

static void ExitCriticalSection(void)
{
    TEST_ASSERT_TRUE(TC_IsCriticalSectionActive);
    TC_IsCriticalSectionActive = false;
}

static void *TC_Alloc(size_t size)
{
    void *p_data = Heap_Alloc(size);
    if (p_data != NULL)
    {
        TEST_ASSERT(((uintptr_t)p_data % _Alignof(max_align_t)) == 0);
        memset(p_data, 0xFF, size);
    }

    return p_data;
}

void setUp(void)
{
    srand(time(NULL));

    TC_IsCriticalSectionActive = false;
    memset(TC_Array, 0x00, sizeof(TC_Array));

    AllocFailedHook_Ignore();
}

void tearDown(void)
{
    TEST_ASSERT_FALSE(TC_IsCriticalSectionActive);
}

void test_fuzz(void)
{
    uint8_t data[1024];
    Heap_Init(&TC_HeapConfig, data, sizeof(data));

    for (size_t i = 0; i < 1000000; i++)
    {
        size_t idx  = rand() % ARRAY_SIZE;
        size_t size = rand() % MAX_ALLOC_SIZE;

        Heap_Free(TC_Array[idx]);
        TC_Array[idx] = TC_Alloc(size);
        TRACE("#%d idx %d: %p\n", i, idx, TC_Array[idx]);
    }

    for (size_t idx = 0; idx < ARRAY_SIZE; idx++)
    {
        Heap_Free(TC_Array[idx]);
    }

    // Try allocating a large chunk to see if everything was freed
    void *p_data = TC_Alloc(sizeof(data) - 50);
    TEST_ASSERT_NOT_NULL(p_data);
}
