add_library(Heap STATIC EXCLUDE_FROM_ALL "./Heap.c")
target_include_directories(Heap PUBLIC "./")

add_subdirectory(unittests)

createCMock(MockAssert ./Assert.h)
target_include_directories(MockAssert PUBLIC ./)