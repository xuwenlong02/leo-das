set(MODULE dbms)

aux_source_directory(. DIRSRCS)
include_directories(
	./
	../include)

add_library(${MODULE} ${DIRSRCS})
target_link_libraries(${MODULE} utils)
