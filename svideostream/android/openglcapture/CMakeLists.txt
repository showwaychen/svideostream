cmake_minimum_required(VERSION 2.6)
project (OPENGLCAPTURE)

#defined										 
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -funwind-tables  -Wno-unused-label -Wno-maybe-uninitialized -Wno-unused-but-set-variable -Wno-deprecated-declarations -Wno-comment -Wno-unused-variable -Wno-sign-compare")
if(${CMAKE_BUILD_TYPE} MATCHES "Release")
    set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -O3")
else()
    set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -g3")
endif()
set(CMAKE_C_FLAGS "-D__STDC_CONSTANT_MACROS")
ADD_DEFINITIONS( -DWEBRTC_ANDROID -DWEBRTC_POSIX)


#project build
file(GLOB  PLAYER_SOURCES_FILES 		${CMAKE_CURRENT_SOURCE_DIR}/*.h
										 ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
										 )							 
add_library( 
             openglcapture
             SHARED
             ${PLAYER_SOURCES_FILES} )
find_library(log-lib
              log )
find_library( 
              GLESv1_CM-lib
              GLESv1_CM )
find_library( 
              GLESv2-lib
              GLESv2 )
find_library( 
              EGL-LIB
              EGL )
target_link_libraries( 
                       openglcapture
                       ${log-lib} ${GLESv1_CM-lib} ${GLESv2-lib} ${EGL-LIB} ${STATICLIBS})
if(${CMAKE_BUILD_TYPE} MATCHES "Release")
    ADD_CUSTOM_COMMAND(TARGET openglcapture
    POST_BUILD
    COMMAND   ${CMAKE_STRIP} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/*.so )		
endif()	
