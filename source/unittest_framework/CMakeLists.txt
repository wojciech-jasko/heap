# Create library with Unity
add_library(unity STATIC unity/src/unity.c)
target_include_directories(unity PUBLIC unity/src)

# Create library with CMock
add_library(cmock STATIC cmock/src/cmock.c)
target_include_directories(cmock PUBLIC unity/src cmock/src)
target_compile_definitions(cmock PUBLIC
    "-DCMOCK_MEM_DYNAMIC"
)

include(ProcessorCount)
ProcessorCount(N)

## Add custom target to force ctest to be more verbose.
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure  -j${N}
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
