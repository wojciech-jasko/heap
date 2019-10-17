#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

/** @brief Heap configuration structure. Implementation of these function shall be implemented by application 
 *         in order to handle error conditions and provide the port of the mutual exclusion mechanism.
 */
struct HeapConfig
{
    /** @brief  Corrupted data hook. It will be notified about the corruption of heap data.
     *          For example, after detection of overflow.
     *
     *  @note   Can not be NULL.
     */
    void (*corrupted_data_hook)(void);

    /** @brief  Allocation failed hook. It notifies about failed allocation.
     *
     *  @note   Can not be NULL.
     */
    void (*alloc_failed_hook)(void);

    /** @brief  Invalid pointer hook. It will be notified about invalid pointer passed to @ref Heap_Free.
     *          For example, after freeing the same memory twice.
     *
     *  @note   Can not be NULL.
     */
    void (*invalid_pointer_hook)(void);

    /** @brief  Enter critical section.
     *
     *  @note   Can not be NULL.
     */
    void (*enter_critical_section)(void);

    /** @brief  Exit critical section.
     *
     *  @note   Can not be NULL.
     */
    void (*exit_critical_section)(void);
};

/** @brief  Initialize heap module.
 *
 *  @note   This function must be called before use of the rest of the module API.
 *
 *  @param [in]      p_config  Model configuration. Can not be NULL.
 *  @param [in]      p_data    Pointer to the static memory dedicated to the heap.
 *  @param [in]      size      Size of the memory in bytes
 */
void  Heap_Init(struct HeapConfig const *p_config, void *p_data, size_t size);

/** @brief  Allocates a block of memory.
 *
 *  @param [in]      size      Size of the requested memory in bytes.
 *
 *  @return Pointer to the beginning of the granted block, NULL in case of failure.
 */
void *Heap_Alloc(size_t size);

/** @brief  Deallocate previously allocated memory
 *
 *  @param [in]      p_data    Pointer to previously allocated memory block.
 */
void  Heap_Free(void *p_data);

#endif /* #ifndef HEAP_H */
