set(MODULE das-server)

aux_source_directory(. DIRSRCS)

include_directories(./)

add_executable(${MODULE} ${DIRSRCS})

message(STATUS "TRANSPORT/PROJECT_BINARY_DIR:       " ${PROJECT_BINARY_DIR})

target_link_libraries(${MODULE} utils jsoncpp transport)

target_link_libraries(${MODULE} ${CMAKE_BINARY_DIR}/thirdparty/Libevent/lib/libevent.a)
target_link_libraries(${MODULE} ${CMAKE_BINARY_DIR}/thirdparty/Libevent/lib/libevent_pthreads.a)
