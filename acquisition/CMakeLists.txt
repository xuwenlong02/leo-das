set(MODULE acquisition)

# -- plugins
#add_subdirectory(plugins)

aux_source_directory(. DIRSRCS)
include_directories(
	./
	../include)

add_library(${MODULE} ${DIRSRCS})
target_link_libraries(${MODULE} utils dbms md5cpp)
