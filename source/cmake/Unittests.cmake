## Find ruby executable.
find_program(CMAKE_RUBY_EXECUTABLE ruby)
if(NOT CMAKE_RUBY_EXECUTABLE)
    message(FATAL_ERROR "Could not find 'ruby' command.")
endif(NOT CMAKE_RUBY_EXECUTABLE)

set(UNITTEST_ARTIFACTS_DIR  test                                                                        CACHE INTERNAL "")
set(CMOCK_GENERATION_SCRIPT ${PROJECT_SOURCE_DIR}/unittest_framework/cmock/lib/cmock.rb                 CACHE INTERNAL "")
set(UNITY_GENERATION_SCRIPT ${PROJECT_SOURCE_DIR}/unittest_framework/unity/auto/generate_test_runner.rb CACHE INTERNAL "")
set(CONFIG_FILE             ${PROJECT_SOURCE_DIR}/unittest_framework/config.yml                         CACHE INTERNAL "")

function(createCMock TARGET SRC)
    get_filename_component(SRC        ${SRC}  REALPATH)
    get_filename_component(FILE_NAME  ${SRC}  NAME_WE)
    set(OUTPUT      Mock${FILE_NAME})

    add_custom_command(OUTPUT   ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}.c ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}.h
                       COMMAND  ${CMAKE_RUBY_EXECUTABLE} ${CMOCK_GENERATION_SCRIPT} -o${CONFIG_FILE} ${SRC}
                       DEPENDS  ${SRC})
    add_library(${TARGET} STATIC ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}.c)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${TARGET} cmock)
endfunction()

function(createTest TARGET TEST_SRC)
    get_filename_component(FILE_NAME  ${TEST_SRC} NAME_WE)
    get_filename_component(SRC        ${TEST_SRC} REALPATH)

    set(RUNNER_SRC  ${FILE_NAME}_Runner.c)

    add_custom_command(OUTPUT   ${RUNNER_SRC}
                       COMMAND  ${CMAKE_RUBY_EXECUTABLE} ${UNITY_GENERATION_SCRIPT} ${CONFIG_FILE} ${SRC} ${RUNNER_SRC}
                       DEPENDS  ${SRC})

    add_executable(${TARGET} ${RUNNER_SRC} ${TEST_SRC} ${ARGN})
    target_link_libraries(${TARGET} unity cmock)

    add_test(NAME ${TARGET} COMMAND ${TARGET})
    add_dependencies(check ${TARGET})
endfunction()