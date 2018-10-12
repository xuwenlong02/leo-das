set(MODULE transport)

aux_source_directory(. DIRSRCS)
aux_source_directory(./local DIRSRCS)
aux_source_directory(./remote DIRSRCS)
aux_source_directory(./remote/server DIRSRCS)
aux_source_directory(./remote/client DIRSRCS)
aux_source_directory(./remote/protocol DIRSRCS)

if(HTTP_SERVER)
    #aux_source_directory(./httpserver DIRSRCS)
endif()

include_directories(
	./
	../include
	../utils)

add_library(${MODULE} ${DIRSRCS})

message(STATUS "TRANSPORT/PROJECT_BINARY_DIR:       " ${PROJECT_BINARY_DIR})

target_link_libraries(${MODULE} utils jsoncpp)

target_link_libraries(${MODULE} ${CMAKE_BINARY_DIR}/thirdparty/Libevent/lib/libevent.a)
target_link_libraries(${MODULE} ${CMAKE_BINARY_DIR}/thirdparty/Libevent/lib/libevent_pthreads.a)


