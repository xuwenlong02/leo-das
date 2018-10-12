set(MODULE watchdog)

if(SYSTEM_ARCH STREQUAL "oldk4")
    set(DIRSRCS watchdog_k4.c check_process.c)
elseif(SYSTEM_ARCH STREQUAL "k2")
#K2,6502
    set(DIRSRCS watchdog_k2.c check_process.c)
elseif(SYSTEM_ARCH STREQUAL "k4")
    set(DIRSRCS watchdog_k4.c check_process.c)
else()
    set(DIRSRCS watchdog_k4.c check_process.c watchdog_k2.c)
endif()

include_directories(
	./
	../include)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "linux")
    add_executable(${MODULE} ${DIRSRCS})
endif()
