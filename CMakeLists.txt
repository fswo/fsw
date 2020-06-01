project(libfsw)

cmake_minimum_required(VERSION 2.8)

enable_language(ASM)
set(FSW_VERSION 0.8.0)
set(FSW_CLFLAGS ssl crypt crypto)
set(CMAKE_CXX_STANDARD 11)
set(ENABLE_DEBUG OFF)
set(ENABLE_TRACE OFF)

# check library
find_library(LIB_SSL ssl)
if(NOT LIB_SSL)
    if (DEFINED openssl_dir)
        include_directories(BEFORE ${openssl_dir}/include)
        link_directories(${openssl_dir}/lib)
    else()
        message(FATAL_ERROR "libssl not found, you should set openssl_dir, eg:
        cmake . -Dopenssl_dir=/usr/local/openssl")
    endif()
endif()

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

file(GLOB_RECURSE HEAD_FILES FOLLOW_SYMLINKS
    ./include/*.h
    thirdparty/http/*.h
    thirdparty/nghttp2/*.h
)

file(GLOB_RECURSE SRC_LIST FOLLOW_SYMLINKS 
    src/*.c
    src/*.cc
    thirdparty/boost/asm/combined.S
    thirdparty/http/http_parser.c
    thirdparty/nghttp2/*.c
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(BEFORE
    ./include
    ./thirdparty/http
    ./thirdparty/nghttp2
)

#shared library
link_directories(${LIBRARY_OUTPUT_PATH})
add_library(libfsw SHARED ${SRC_LIST})
set_target_properties(libfsw PROPERTIES OUTPUT_NAME "fsw" VERSION ${FSW_VERSION})
target_link_libraries(libfsw ${FSW_CLFLAGS})

#install
install(CODE "MESSAGE(\"Are you run command using root user?\")")
install(TARGETS libfsw LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${HEAD_FILES} DESTINATION include/fsw)
