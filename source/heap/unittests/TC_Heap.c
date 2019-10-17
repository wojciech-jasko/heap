#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "Heap.h"
#include "MockHeapHelper.h"
#include "MockAssert.h"
#include "unity.h"

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

static bool TC_IsCriticalSectionActive;

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

static void TC_Init256(void)
{
    static uint8_t data[256];

    Heap_Init(&TC_HeapConfig, data, sizeof(data));
}

static void TC_InitUnaligned(void)
{
    static uint8_t data[256];
    size_t padding = (((uintptr_t)data % 2) == 0) ? 1 : 0;

    Heap_Init(&TC_HeapConfig, data + padding, sizeof(data) - padding);
}

static void *TC_AllocSuccess(size_t size)
{
    void *p_data = Heap_Alloc(size);
    TEST_ASSERT_NOT_NULL(p_data);
    TEST_ASSERT(((uintptr_t)p_data % _Alignof(max_align_t)) == 0);

    return p_data;
}

static void TC_AllocFailed(size_t size)
{
    AllocFailedHook_Expect();

    void *p_data = Heap_Alloc(size);
    TEST_ASSERT_NULL(p_data);
}

void setUp(void)
{
    TC_IsCriticalSectionActive = false;
}

void tearDown(void)
{
    TEST_ASSERT_FALSE(TC_IsCriticalSectionActive);
}

void test_single_allocation(void)
{
    TC_Init256();

    size_t size   = 10;
    void * p_data = TC_AllocSuccess(size);

    memset(p_data, 0xFF, size);
    Heap_Free(p_data);
}

void test_allocation_0_size(void)
{
    TC_Init256();

    void * p_data = TC_AllocSuccess(0);
    Heap_Free(p_data);
}

void test_multiple_allocation_of_const_size(void)
{
    TC_Init256();

    for (size_t i = 0; i < 5; i++)
    {
        size_t size   = 10;
        void * p_data = TC_AllocSuccess(size);

        memset(p_data, 0xFF, size);
        Heap_Free(p_data);
    }
}

void test_module_can_be_initialized_from_not_aligned_data(void)
{
    TC_InitUnaligned();

    TC_AllocSuccess(10);
}

void test_too_big_allocation_shall_fail(void)
{
    TC_Init256();

    TC_AllocFailed(512);
}

void test_multiple_allocations_in_row(void)
{
    TC_Init256();

    void *p_data_0 = TC_AllocSuccess(10);
    void *p_data_1 = TC_AllocSuccess(10);

    Heap_Free(p_data_0);
    Heap_Free(p_data_1);
}

void test_multiple_allocations_in_row_with_reverse_free(void)
{
    TC_Init256();

    void *p_data_0 = TC_AllocSuccess(10);
    void *p_data_1 = TC_AllocSuccess(10);

    Heap_Free(p_data_1);
    Heap_Free(p_data_0);
}

void test_forward_overflow(void)
{
    TC_Init256();

    size_t size   = 10;
    void * p_data = TC_AllocSuccess(size);

    memset(p_data, 0xFF, size + _Alignof(max_align_t));

    CorruptedDataHook_Expect();
    Heap_Free(p_data);
}

void test_backward_overflow(void)
{
    TC_Init256();

    /* Make sure that overflow will not case Seg fault*/
    TC_AllocSuccess(40);

    size_t size   = 10;
    void * p_data = TC_AllocSuccess(size);

    uint8_t *p_temp = (uint8_t *)p_data - _Alignof(max_align_t);
    memset(p_temp, 0xFF, size);

    CorruptedDataHook_Expect();
    Heap_Free(p_data);
}

void test_freeing_twice_the_same_pointer_shall_fail(void)
{
    TC_Init256();

    void *p_data_0 = TC_AllocSuccess(10);
    void *p_data_1 = TC_AllocSuccess(10);

    (void)p_data_1;
    Heap_Free(p_data_0);

    InvalidPointerHook_Expect();
    Heap_Free(p_data_0);
}
