#rights reserved
cmake_minimum_required(VERSION 2.8)

#project name
set(PROJECT_NAME "leo-das")
option(TEST "test mode" OFF)
option(HTTP_SERVER "http_server" ON)
#option(SYSTEM_ARCH "SYSTEM ARCH" "gnu")

message(INFO " SYSTEM_ARCH:${SYSTEM_ARCH}")

#set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS "-fPIC -std=gnu++0x")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
     add_definitions(-DLOCAL_TEST)
     link_directories(3rdlibs/libs/linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "arm-linux")
#老版K4,6857
    add_definitions(-DUSE_HTNICE)
    if(SYSTEM_ARCH STREQUAL "oldk4")
        set(COMPILER_ROOT "/usr/local/arm/4.5.1")
        add_definitions(-DHTNICE_OLDK4)
    elseif(SYSTEM_ARCH STREQUAL "k2")
#K2,6502
        add_definitions(-DHTNICE_K2)
        set(COMPILER_ROOT "/usr/local/arm/4.8.3")
    else()
        add_definitions(-DHTNICE_K2)
        set(COMPILER_ROOT "/usr/local/arm/4.8.3")
    endif()

    set(CMAKE_C_COMPILER "${COMPILER_ROOT}/bin/arm-linux-gcc")
    set(CMAKE_CXX_COMPILER "${COMPILER_ROOT}/bin/arm-linux-g++")
    message(INFO " C compiler is ${CMAKE_C_COMPILER}")
    message(INFO " CXX compiler is ${CMAKE_CXX_COMPILER}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "arm-linux-gnueabihf")
#K4,6657
    add_definitions(-DHTNICE_K4)
    add_definitions(-DUSE_HTNICE)
    set(CMAKE_SYSROOT /opt/rootfs_6657)
    if (NOT COMPILER_ROOT)
        set(COMPILER_ROOT "/usr/local/arm/arm6657")
    endif()
    set(CMAKE_C_COMPILER "${COMPILER_ROOT}/bin/arm-linux-gnueabihf-gcc")
    set(CMAKE_CXX_COMPILER "${COMPILER_ROOT}/bin/arm-linux-gnueabihf-g++")
    message(INFO " C compiler is ${CMAKE_C_COMPILER}")
    message(INFO " CXX compiler is ${CMAKE_CXX_COMPILER}")
else()
endif()

if(TEST)
    add_definitions(-DRUN_TEST)
endif()

if(HTTP_SERVER)
    add_definitions(-DHTTP_SERVER)
endif()

#设置相对路径
set(CMAKE_SUPPRESS_REGENERATION 1)
set(CMAKE_USE_RELATIVE_PATHS ON)

project(${PROJECT_NAME})

## -- link libs
cmake_policy(PUSH)
cmake_policy(SET CMP0015 NEW)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DLOCAL_TEST)
    link_directories(3rdlibs/libs/linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "arm-linux")
    link_directories(3rdlibs/libs/arm-linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "arm-linux-gnueabihf")
    link_directories(3rdlibs/libs/armhf-linux)
endif()
if(HTTP_SERVER)
link_directories(${PROJECT_BINARY_DIR}/thirdparty/Libevent/lib)
include_directories(thirdparty/Libevent/include ${CMAKE_BINARY_DIR}/thirdparty/Libevent/include)
endif()
cmake_policy(POP)

#

include_directories(include thirdparty 3rdlibs/include ./ thirdparty/jsoncpp/include)
aux_source_directory(. DIR_SRCS)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
add_subdirectory(thirdparty)
add_subdirectory(utils)
add_subdirectory(dbms)
add_subdirectory(transport)
add_subdirectory(devicemanager)
add_subdirectory(baseinfo)
add_subdirectory(acquisition)
add_subdirectory(remoteservice)
add_subdirectory(dasclient)
add_subdirectory(dasserver)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
add_subdirectory(watchdog)
endif()

if(HTTP_SERVER)
#add_subdirectory(httpserver)
endif()

if(TEST)
    add_subdirectory(test)
endif()

#add executable 
add_executable(${PROJECT_NAME} ${DIR_SRCS})
target_link_libraries(${PROJECT_NAME} transport acquisition devicemanager)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(
    FILES config/Config.ini config/das.db
    DESTINATION config
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
    FILES das-run.sh das-stop.sh log-check.sh
    DESTINATION bin
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
    FILES install.sh
    DESTINATION .
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)
