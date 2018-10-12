set(MODULE remoteservice)

aux_source_directory(. DIRSRCS)
include_directories(
	./
	../include)

add_library(${MODULE} ${DIRSRCS})
