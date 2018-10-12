include_directories(./)

# --- cJson

#set(ENABLE_CJSON_UTILS OFF)
#set(ENABLE_CJSON_TEST OFF)
#set(CJSON_BUILD_SHARED_LIBS OFF)
#set(CJSON_OVERRIDE_BUILD_SHARED_LIBS OFF)
#set(BUILD_SHARED_LIBS OFF)
#set(BUILD_SHARED_AND_STATIC_LIBS OFF)
#add_subdirectory(cJSON)

# --- jsoncpp
add_subdirectory(jsoncpp)

# --- mxml-2.5
#add_subdirectory(mxml)

# --- sqlite3pp
#add_subdirectory(sqlite3pp)

# --- md5cpp
add_subdirectory(md5cpp)

#--- inifile2
add_subdirectory(inifile2)

# --- libevent
#set(CURRENT_PREFIX ${CMAKE_INSTALL_PREFIX})
#set(CMAKE_INSTALL_PREFIX  "${CMAKE_CURRENT_BINARY_DIR}/Libevent/install")
if(HTTP_SERVER)
add_subdirectory(Libevent)
#set(CMAKE_INSTALL_PREFIX ${CURRENT_PREFIX})
endif()

# --- cpp-base64
add_subdirectory(cpp-base64)
