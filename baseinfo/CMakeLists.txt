set(MODULE baseinfo)

aux_source_directory(. DIRSRCS)
include_directories(
	./
        ../include
        ../thirdparty/jsoncpp/include)

add_library(${MODULE} ${DIRSRCS})
target_link_libraries(${MODULE} jsoncpp md5cpp transport)
