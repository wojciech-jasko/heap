createCMock(MockHeapHelper ./HeapHelper.h)
target_include_directories(MockHeapHelper PUBLIC ./)

createTest(test_Heap ./TC_Heap.c)
target_link_libraries(test_Heap Heap MockHeapHelper MockAssert)

createTest(test_FuzzHeap ./TC_FuzzHeap.c)
target_link_libraries(test_FuzzHeap Heap MockHeapHelper MockAssert)