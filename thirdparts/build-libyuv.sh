#!/bin/bash

if [ x$NDKROOT = "x" ]
then 
    echo ERROR:NDKROOT env variable is not set
	exit 1;
fi
# Environment
pushd ./
cd ./libyuv/jni
$NDKROOT/ndk-build clean
$NDKROOT/ndk-build
popd
ABIS=('armeabi' 'armeabi-v7a' 'x86' 'mips' 'arm64-v8a' 'x86_64')
for ABI in ${ABIS[@]}
do
	dst_dir=./build/${ABI}/libyuv/lib
	mkdir -p ${dst_dir}
	cp -f ./libyuv/obj/local/${ABI}/libyuv.a ${dst_dir}
done

rm -rf ./libyuv/obj


