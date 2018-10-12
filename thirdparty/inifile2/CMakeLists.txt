set(MODULE inifile2)

# -- plugins
#add_subdirectory(plugins)

aux_source_directory(./src/ DIRSRCS)
include_directories(
	./src/
	)

add_library(${MODULE} ${DIRSRCS})
