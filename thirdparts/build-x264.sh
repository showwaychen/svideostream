#!/bin/bash
FF_ARCH="all32"

if [ $# -lt 1 ]; then
	echo "-->not specify a architecture, use 'all32' as default"
elif [ $# -gt 1 ]; then
	echo "-->too many architectures arguments, architecture must be one of 'armv5|armv7a|arm32|arm64|arm|x86|x86_64|mips|mips64|all32|all64|all'"
	exit 1
else
	FF_ARCH=$1
	echo "-->specify architecture : $FF_ARCH"
fi


if [ x$NDKROOT = "x" ]
then 
    echo ERROR:NDKROOT env variable is not set
	exit 1;
fi

NDK=$NDKROOT
LOCAL_PATH=`pwd`
BUILD_PATH=$LOCAL_PATH/build
MODULE="x264"
NDK_UNAME=`uname -s | tr '[A-Z]' '[a-z]'`
CROSS_PREFIX=
TOOLCHAIN=
ABI=
PLATFORM=
PREFIX=
EXTRA_CFLAGS=
HOST=

function build ()
{
rm -rf ${PREFIX}
cd x264
chmod +x configure *.sh
make clean
./configure \
  --prefix=$PREFIX \
  --enable-static \
  --disable-cli \
  --enable-pic \
  --enable-strip \
  --host=${HOST} \
  --extra-cflags="${EXTRA_CFLAGS}" \
  --cross-prefix=$TOOLCHAIN/bin/${CROSS_PREFIX}- \
  --sysroot=$PLATFORM
make clean
make
make install
cd -
}

function build_armv5 ()
{
echo "build armv5"
ABI="armeabi"
FF_ARCH=arm
CROSS_PREFIX=arm-linux-androideabi
HOST=arm-linux
PLATFORM=$NDK/platforms/android-15/arch-${FF_ARCH}/
TOOLCHAIN=$NDK/toolchains/${CROSS_PREFIX}-4.9/prebuilt/${NDK_UNAME}-x86_64
PREFIX=$BUILD_PATH/${ABI}/${MODULE}
EXTRA_CFLAGS=
build
echo "build armv5 end"
}

function build_armv7a ()
{
echo "build armv7a"
ABI="armeabi-v7a"
FF_ARCH=arm
CROSS_PREFIX=arm-linux-androideabi
HOST=arm-linux
PLATFORM=$NDK/platforms/android-15/arch-${FF_ARCH}/
TOOLCHAIN=$NDK/toolchains/${CROSS_PREFIX}-4.9/prebuilt/${NDK_UNAME}-x86_64
PREFIX=$BUILD_PATH/${ABI}/${MODULE}
EXTRA_CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__"
build
echo "build armv7a end"
}

function build_arm64-v8a ()
{
echo "build arm64-v8a"
ABI="arm64-v8a"
FF_ARCH=arm64
CROSS_PREFIX=aarch64-linux-android
HOST=aarch64-linux
PLATFORM=$NDK/platforms/android-21/arch-${FF_ARCH}/
TOOLCHAIN=$NDK/toolchains/${CROSS_PREFIX}-4.9/prebuilt/${NDK_UNAME}-x86_64
PREFIX=$BUILD_PATH/${ABI}/${MODULE}
build
echo "build arm64-v8a end"
}

function build_arm32 ()
{
build_armv5
build_armv7a
}

function build_arm64 ()
{
build_arm64-v8a
}
function build_arm ()
{
build_arm32
build_arm64
}

function build_x86 ()
{
echo "build x86"
ABI="x86"
FF_ARCH=x86
CROSS_PREFIX=i686-linux-android
HOST=i686-linux
PLATFORM=$NDK/platforms/android-15/arch-${FF_ARCH}/
TOOLCHAIN=$NDK/toolchains/${CROSS_PREFIX}-4.9/prebuilt/${NDK_UNAME}-x86_64
PREFIX=$BUILD_PATH/${ABI}/${MODULE}
build
echo "build x86 end"
}
function build_x86_64 ()
{
echo "build x86_64"
ABI="x86_64"
FF_ARCH=x86_64
CROSS_PREFIX=x86_64-linux-android
HOST=x86_64-linux
PLATFORM=$NDK/platforms/android-21/arch-${FF_ARCH}/
TOOLCHAIN=$NDK/toolchains/${CROSS_PREFIX}-4.9/prebuilt/${NDK_UNAME}-x86_64
PREFIX=$BUILD_PATH/${ABI}/${MODULE}
build
echo "build x86_64 end"
}
function build_mips ()
{
echo "build mips"
ABI="mips"
FF_ARCH=mips
CROSS_PREFIX=mipsel-linux-android
HOST=mipsel-linux
PLATFORM=$NDK/platforms/android-21/arch-${FF_ARCH}/
TOOLCHAIN=$NDK/toolchains/${CROSS_PREFIX}-4.9/prebuilt/${NDK_UNAME}-x86_64
PREFIX=$BUILD_PATH/${ABI}/${MODULE}
build
echo "build mips64 end"
}

function build_all32 ()
{
build_arm32
build_x86
build_mips
}


function build_all64 ()
{
build_arm64
build_x86_64
build_mips64
}


function build_all ()
{
build_all32
build_all64
}

case $FF_ARCH in
	armv5)
	echo "****** build armv5 ******"
	build_armv5
	;;
	armv7a)
	echo "****** build armv7a ******"
	build_armv7a
	;;
	arm32)
	echo "****** build armv5 armv7a ******"
	build_arm32
	;;
	arm64)
	echo "****** build arm64-v8a ******"
	build_arm64
	;;
	arm)
	echo "****** build armv5 armv7a arm64-v8a ******"
	build_arm
	;;
	x86)
	echo "****** build x86 ******"
	build_x86
	;;
	x86_64)
	echo "****** build x86_64 ******"
	build_x86_64
	;;
	mips)
	echo "****** build mips ******"
	build_mips
	;;
	all32)
	echo "****** build all32(armv5 armv7a x86 mips) ******"
	build_all32
	;;
	all64)
	echo "****** build all64(arm74-v8a x86_64) ******"
	build_all64
	;;
	all)
	echo "****** build all(armv5 armv7a arm64-v8a x86 x86_64 mips) ******"
	build_all
	;;
	*)
	echo "-->architecture:$FF_ARCH is not supported, architecture must be one of 'armv5|armv7a|arm32|arm64|arm|x86|x86_64|mips|all32|all64|all'"
	;;
esac