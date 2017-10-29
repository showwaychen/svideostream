#!/bin/bash

if [ x$NDKROOT = "x" ]
then 
    echo ERROR:NDKROOT env variable is not set
	exit 1;
fi
# Environment
pushd ./
cd ./librtmp/jni
$NDKROOT/ndk-build clean
$NDKROOT/ndk-build
popd
ABIS=('armeabi' 'armeabi-v7a' 'x86' 'mips' 'arm64-v8a' 'x86_64')
for ABI in ${ABIS[@]}
do
	dst_dir=./build/${ABI}/librtmp/lib
	mkdir -p ${dst_dir}
	cp -f ./librtmp/obj/local/${ABI}/librtmp.a ${dst_dir}
done

rm -rf ./librtmp/obj


