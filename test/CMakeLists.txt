set(MODULE test)

aux_source_directory(. DIRSRCS)
include_directories(
	./
	../include)

add_executable("test" ${DIRSRCS})
target_link_libraries("test" transport utils)
