#include "Heap.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Assert.h"

/** @brief  System alignmet. It shall be defined by port if _Alignof keyword is not available.
 */
#ifndef HEAP_ALIGNMENT
#define HEAP_ALIGNMENT _Alignof(max_align_t)
#endif

/** @brief  Size of watermark used to guard boundaries of allocated block.
 */
#ifndef HEAP_WATERMARK_SIZE
#define HEAP_WATERMARK_SIZE (4U)
#endif

/** @brief  Block forward declaration.
 */
struct Block;

/** @brief  Free memory block linked list.
 *
 *  @note Blocks are linked in order of their memory address.
 */
struct Block
{
    struct Block *p_next;      /**< Next memory block. NULL if it is last free block. */
    size_t        total_size;  /**< Size of the memory (including this structure). */
};

/*  Blocks memory layout
    ______________________________________________________________________________________
    | struct Block | HEAD_WATERMARK | padding |Data | unused | TAIL_WATERMARK | Next Block
    |______________|________________|_________|_____|________|________________|___________
    |                                         |
    | aligned                                 | aligned
 */

/** @brief  Heap module control structure.
 */
struct Heap
{
    struct HeapConfig config;   /**< Application define configuration. */
    struct Block      head;     /**< Head of the block linked list. */
};

/** @brief  Heap control structure instance.
 */
static struct Heap HeapInstance;

/** @brief  Magic value used to mark boundaries of allocated block.
 */
static const uint8_t WATERMARK = 0xAA;

/** @brief  Returns the offset enlarged to the nearest multiple of @ref HEAP_ALIGNMENT.
 */
static size_t AddPadding(size_t offset);

/** @brief  Get padding size required to align the passed pointer.
 */
static size_t GetPaddingSizeFromPointer(void const *p_data);

/** @brief  Get offset of data from the begining of block. Please see @ref Blocks memory layout.
 */
static size_t GetOffsetOfData(void);

/** @brief  Get minimal total block size (together with struct Block, paddings and watermarks)
 *          required to hold data of the given size. Please see @ref Blocks memory layout.
 */
static size_t GetTotalSize(size_t data_size);

/** @brief  Get offset of head guard from the begining of block. Please see @ref Blocks memory layout.
 */
static size_t GetOffsetOfHeadGuard(void);

/** @brief  Get offset of tail guard from the begining of block. Please see @ref Blocks memory layout.
 */
static size_t GetOffsetOfTailGuard(size_t block_size);

/** @brief  Apply watermarks to the block.
 */
static void ApplyWatermarks(struct Block *p_block);

/** @brief  Checks watermarks of the block.
 */
static bool CheckWatermarks(struct Block const* p_block);

/** @brief  Find the first free block of size equal or greater than given total_size and return the previous one.
 *          NULL in case of failure.
 */
static struct Block* FindBlockThatPrecedesFreeOne(size_t total_size);

/** @brief  Find the previous block to the given in the free block list.
 */
static struct Block* FindPrecendingBlock(struct Block const *p_block);

/** @brief  Try to split the given block. It makes sure that the given block will have at least @ref total_size.
 */
static void TryToSplitBlock(struct Block* p_block, size_t total_size);

/** @brief  Try to merge the given block with the next one (Only when they are adjacent).
 *  @note   The next block may be destroyed.
 */
static void TryToMergeWithNextBlock(struct Block* p_block);


void Heap_Init(struct HeapConfig const *p_config, void *p_data, size_t size)
{
    ASSERT(p_data != NULL);
    ASSERT(p_config != NULL);
    ASSERT(p_config->invalid_pointer_hook != NULL);
    ASSERT(p_config->alloc_failed_hook != NULL);
    ASSERT(p_config->corrupted_data_hook != NULL);
    ASSERT(p_config->enter_critical_section != NULL);
    ASSERT(p_config->exit_critical_section != NULL);

    memcpy(&HeapInstance.config, p_config, sizeof(HeapInstance.config));

    /* Used memory shall be aligned. */
    size_t padding = GetPaddingSizeFromPointer(p_data);

    ASSERT(padding <= size);
    size_t aligned_size = size - padding;

    size_t min_size = GetTotalSize(0);
    ASSERT(min_size <= aligned_size);

    struct Block *p_first = (struct Block *)((uintptr_t)p_data + padding);
    p_first->p_next       = NULL;
    p_first->total_size   = aligned_size;

    /* Head is not used for allocation (total_size of 0 prevents the algorithm from choosing it). */
    HeapInstance.head.total_size = 0;
    HeapInstance.head.p_next     = p_first;

    return;
}


void *Heap_Alloc(size_t size)
{
    struct Block* p_free     = NULL;
    size_t        total_size = GetTotalSize(size);

    HeapInstance.config.enter_critical_section();

    struct Block* p_prev = FindBlockThatPrecedesFreeOne(total_size);
    if (p_prev != NULL)
    {
        p_free = p_prev->p_next;
        ASSERT(p_free != NULL);

        TryToSplitBlock(p_free, total_size);

        /* Remove from list. */
        p_prev->p_next = p_free->p_next;
    }

    HeapInstance.config.exit_critical_section();

    if (p_free == NULL)
    {
        HeapInstance.config.alloc_failed_hook();
        return NULL;
    }

    /* Mark as used. */
    p_free->p_next = NULL;
    ApplyWatermarks(p_free);

    return (void *)((uintptr_t)p_free + GetOffsetOfData());
}


void Heap_Free(void *p_data)
{
    if (p_data == NULL)
    {
        return;
    }

    struct Block* p_block = (struct Block *)((uintptr_t)p_data - GetOffsetOfData());

    if (GetPaddingSizeFromPointer(p_block) != 0)
    {
        HeapInstance.config.invalid_pointer_hook();
        return;
    }

    if (p_block->p_next != NULL)
    {
        /* Block is not allocated. */
        HeapInstance.config.invalid_pointer_hook();
        return;
    }

    if (!CheckWatermarks(p_block))
    {
        HeapInstance.config.corrupted_data_hook();
        return;
    }

    HeapInstance.config.enter_critical_section();

    struct Block* p_prev = FindPrecendingBlock(p_block);
    if (p_prev != NULL)
    {
        /* Insert to list. */
        p_block->p_next = p_prev->p_next;
        p_prev->p_next  = p_block;

        /* Try to combine adjacent memory blocks.
         * Note that this function may destroy next block. That is why order is important.
         */
        TryToMergeWithNextBlock(p_block);
        TryToMergeWithNextBlock(p_prev);
    }

    HeapInstance.config.exit_critical_section();

    if (p_prev == NULL)
    {
        /* Hooks shall not be call within critical section. */
        HeapInstance.config.invalid_pointer_hook();
    }

    return;
}


static size_t AddPadding(size_t offset)
{
    size_t remainder = (offset % HEAP_ALIGNMENT);
    size_t padding  = (remainder != 0) ? (HEAP_ALIGNMENT - remainder) : 0;
    return offset + padding;
}


static size_t GetPaddingSizeFromPointer(void const *p_data)
{
    size_t remainder = ((uintptr_t)p_data % HEAP_ALIGNMENT);
    return (remainder != 0) ? (HEAP_ALIGNMENT - remainder) : 0;
}


static size_t GetOffsetOfData(void)
{
    size_t fields_size = sizeof(struct Block) + HEAP_WATERMARK_SIZE;
    return AddPadding(fields_size);
}


static size_t GetTotalSize(size_t data_size)
{
    return GetOffsetOfData() + data_size + HEAP_WATERMARK_SIZE;
}


static size_t GetOffsetOfHeadGuard(void)
{
    return sizeof(struct Block);
}


static size_t GetOffsetOfTailGuard(size_t block_size)
{
    return block_size - HEAP_WATERMARK_SIZE;
}


static void ApplyWatermarks(struct Block *p_block)
{
    uint8_t *p_head = (uint8_t *)p_block + GetOffsetOfHeadGuard();
    for (size_t i = 0; i < HEAP_WATERMARK_SIZE; i++)
    {
        p_head[i] = WATERMARK;
    }

    uint8_t *p_tail = (uint8_t *)p_block + GetOffsetOfTailGuard(p_block->total_size);
    for (size_t i = 0; i < HEAP_WATERMARK_SIZE; i++)
    {
        p_tail[i] = WATERMARK;
    }

    return;
}


static bool CheckWatermarks(struct Block const *p_block)
{
    uint8_t *p_head = (uint8_t *)p_block + GetOffsetOfHeadGuard();
    for (size_t i = 0; i < HEAP_WATERMARK_SIZE; i++)
    {
        if (p_head[i] != WATERMARK)
        {
            return false;
        }
    }

    uint8_t *p_tail = (uint8_t *)p_block + GetOffsetOfTailGuard(p_block->total_size);
    for (size_t i = 0; i < HEAP_WATERMARK_SIZE; i++)
    {
        if (p_tail[i] != WATERMARK)
        {
            return false;
        }
    }
    return true;
}


static struct Block* FindBlockThatPrecedesFreeOne(size_t total_size)
{
    for (struct Block *p_it = &HeapInstance.head; p_it->p_next != NULL; p_it = p_it->p_next)
    {
        struct Block *p_next = p_it->p_next;

        if (p_next->total_size >= total_size)
        {
            return p_it;
        }
    }
    return NULL;
}


static struct Block* FindPrecendingBlock(struct Block const* p_block)
{
    struct Block *p_it = &HeapInstance.head;

    for (p_it = &HeapInstance.head; p_it->p_next != NULL; p_it = p_it->p_next)
    {
        if (p_it->p_next >= p_block)
        {
            break;
        }
    }
    return p_it;
}


static void TryToSplitBlock(struct Block* p_block, size_t total_size)
{
    /* New block shall be corretly aligned. */
    size_t aligned_size = AddPadding(total_size);
    size_t min_size     = GetTotalSize(0);

    /* Note that HeapInstance.head never passes this condition. */
    if (p_block->total_size > aligned_size + min_size)
    {
        size_t new_block_size = p_block->total_size - aligned_size;

        p_block->total_size = aligned_size;

        struct Block *p_new = (struct Block *)((uintptr_t)p_block + p_block->total_size);

        p_new->total_size = new_block_size;
        p_new->p_next     = p_block->p_next;
        p_block->p_next   = p_new;
    }

    return;
}


static void TryToMergeWithNextBlock(struct Block* p_block)
{
    struct Block* p_next = p_block->p_next;

    if (((uintptr_t)p_block + p_block->total_size) == (uintptr_t)p_next)
    {
        ASSERT(p_next != NULL);

        p_block->p_next       = p_next->p_next;
        p_block->total_size  += p_next->total_size;
    }

    return;
}
