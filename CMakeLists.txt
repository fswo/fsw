project(libfsw)

cmake_minimum_required(VERSION 2.8)

enable_language(ASM)
set(FSW_VERSION 0.0.1)
# set(FSW_CLFLAGS pthread uv)
set(CMAKE_CXX_STANDARD 11)
set(ENABLE_DEBUG OFF)
set(ENABLE_TRACE OFF)

include(CheckFunctionExists)
check_function_exists(epoll_create HAVE_EPOLL)

option(ENABLE_DEBUGGER "Enable debug" ${ENABLE_DEBUG})
if (ENABLE_DEBUGGER)
    set(ENABLE_DEBUGGER 1)
    message ("-- Enable debug: yes")
else()
    set(ENABLE_DEBUGGER 0)
    message ("-- Enable debug: no")
endif()

option(ENABLE_TRACE "Enable trace" ${ENABLE_TRACE})
if (ENABLE_TRACE)
    set(ENABLE_TRACE 1)
    message ("-- Enable trace: yes")
else()
    set(ENABLE_TRACE 0)
    message ("-- Enable trace: no")
endif()

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

file(GLOB_RECURSE SRC_LIST FOLLOW_SYMLINKS src/*.c src/*.cc thirdparty/boost/asm/combined.S thirdparty/http/http_parser.c)
file(GLOB_RECURSE HEAD_FILES FOLLOW_SYMLINKS include/*.h thirdparty/http/*.h)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(BEFORE ./include ./thirdparty/http)

#shared library
add_library(shared SHARED ${SRC_LIST})
set_target_properties(shared PROPERTIES OUTPUT_NAME "fsw" VERSION ${FSW_VERSION})
target_link_libraries(shared ${FSW_CLFLAGS})

link_directories(${LIBRARY_OUTPUT_PATH})

#install
install(CODE "MESSAGE(\"Are you run command using root user?\")")
install(TARGETS shared LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${HEAD_FILES} DESTINATION include/fsw)

# build test
file(GLOB_RECURSE TEST_LIST test/*.cc)
add_executable(test ${TEST_LIST})
target_link_libraries(test -lgtest -lfsw)