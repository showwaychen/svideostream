### svideostream

#  a android video recording and living library

### Features
    cpu: ARMv7a
    camera recording
	camera rtmp living
	H264: x264 Android MediaCodec
	AAC: fdkaac

###  Build
    export NDKROOT=  your ndk root path
	
    cd thirdparts
    ./download-baselib.sh
    ./download-ffmpeg.sh
	./download-fdkaac.sh
	./download-x264.sh
    ./build-fdk.sh
	./build-librtmp.sh
	./build-libyuv.sh
	./build-x264.sh
    ./build-baselib.sh
	./build-ffmpeg.sh

   
    cd svideostream/build
    ./build.sh
    make -j4

    output directory is svideostream/libs/armeabi-v7a
