set(MODULE utils)

aux_source_directory(. DIRSRCS)
aux_source_directory(./threads/ DIRSRCS)
#aux_source_directory(./worker/ DIRSRCS)
include_directories(
	./
	../include)

add_library(${MODULE} ${DIRSRCS})
target_link_libraries(${MODULE} pthread rt sqlite3 inifile2)
target_link_libraries(${MODULE} ${CMAKE_BINARY_DIR}/thirdparty/Libevent/lib/libevent.a)
target_link_libraries(${MODULE} ${CMAKE_BINARY_DIR}/thirdparty/Libevent/lib/libevent_pthreads.a)
