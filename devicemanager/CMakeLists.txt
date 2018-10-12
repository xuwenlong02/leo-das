set(MODULE devicemanager)

aux_source_directory(. DIRSRCS)
include_directories(
	./
	../include)

add_library(${MODULE} ${DIRSRCS})
target_link_libraries(${MODULE} sqlite3 acquisition dbms)
