include_directories (
  ./include/
)

set (SOURCES md5.cpp
)

add_library("md5cpp" ${SOURCES})
