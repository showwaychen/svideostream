cmake_minimum_required(VERSION 2.6)

#set(CMAKE_SYSTEM_NAME Android)
#set(CMAKE_SYSTEM_VERSION 21) # API level
##set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
#set(CMAKE_ANDROID_NDK /path/to/android-ndk)
#set(CMAKE_ANDROID_STL_TYPE gnustl_static)
project (SVIDEOSTREAM)



#defined										 
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -funwind-tables  -Wno-unused-label -Wno-maybe-uninitialized -Wno-unused-but-set-variable -Wno-deprecated-declarations -Wno-comment -Wno-unused-variable -Wno-sign-compare")
if(${CMAKE_BUILD_TYPE} MATCHES "Release")
    set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -O3")
else()
    set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -g3")
endif()
set(CMAKE_C_FLAGS "-D__STDC_CONSTANT_MACROS")
ADD_DEFINITIONS( -DWEBRTC_ANDROID -DWEBRTC_POSIX)
###includes			
set(ALL_LIB_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparts/build/${ANDROID_ABI})	
#baselib
set(BASELIBROOT_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparts/baselib)
set(BASELIB_INCLUDEDIR ${BASELIBROOT_INCLUDEDIR}/webrtc)
set(BASELIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparts/baselib/android/lib/)
#ffmpeg
set(FFMPEG_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparts/ffmpeg)
#x264
set(X264_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparts/x264)
#fdk-aac
set(FDKAAC_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparts/build/${ANDROID_ABI}/fdk/include/fdk-aac)
#libyuv
set(LIBYUV_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparts/libyuv/jni/include)
#libyuv
set(LIBRTMP_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparts/librtmp/jni)
include_directories("${FFMPEG_INCLUDEDIR}" "${BASELIB_INCLUDEDIR}" "${BASELIBROOT_INCLUDEDIR}" "${X264_INCLUDEDIR}" "${FDKAAC_INCLUDEDIR}" "${LIBYUV_INCLUDEDIR}" "${LIBRTMP_INCLUDEDIR}")			

###library
#ffmpge	
set(FFMPEGLIB ${ALL_LIB_PATH}/ffmpeg/lib/libavformat.a 
				${ALL_LIB_PATH}/ffmpeg/lib/libavcodec.a 
				${ALL_LIB_PATH}/ffmpeg/lib/libavutil.a 
				)
#baselib
set(BASELIB_LIB  ${BASELIB_PATH}/libbaselib.a)		
#x264
set(X264_LIB  ${ALL_LIB_PATH}/x264/lib/libx264.a)		
#fdk-aac
set(FDKAAC_LIB  ${ALL_LIB_PATH}/fdk/lib/libfdk-aac.a)
#libyuv
set(LIBYUV_LIB  ${ALL_LIB_PATH}/libyuv/lib/libyuv.a)

#libyuv
set(LIBRTMP_LIB  ${ALL_LIB_PATH}/librtmp/lib/librtmp.a)
set (STATICLIBS ${FFMPEGLIB} ${BASELIB_LIB} ${X264_LIB} ${FDKAAC_LIB} ${LIBYUV_LIB} ${LIBRTMP_LIB})

#project build
file(GLOB  PLAYER_SOURCES_FILES 		${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
										 ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
										 ${CMAKE_CURRENT_SOURCE_DIR}/android/*.cpp 
										 ${CMAKE_CURRENT_SOURCE_DIR}/android/*.h
										 )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI})										 
add_library( 
             svideostream
             SHARED
             ${PLAYER_SOURCES_FILES} )
find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )	  
target_link_libraries( 
                       svideostream
                       ${log-lib} ${STATICLIBS})
if(${CMAKE_BUILD_TYPE} MATCHES "Release")
    ADD_CUSTOM_COMMAND(TARGET svideostream
    POST_BUILD
    COMMAND   ${CMAKE_STRIP} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/*.so )		
endif()			
			   
#install(TARGETS svplayer
#                LIBRARY DESTINATION lib)					
