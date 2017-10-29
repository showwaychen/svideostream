
if [ x$NDKROOT = "x" ]
then 
    echo ERROR:NDKROOT env variable is not set
	exit 1;
fi
AAC_ARCH="all32"

if [ $# -lt 1 ]; then
	echo "-->not specify a architecture, use 'all32' as default"
elif [ $# -gt 1 ]; then
	echo "-->too many architectures arguments, architecture must be one of 'armv5|armv7a|arm32|arm64|arm|x86|x86_64|mips|mips64|all32|all64|all'"
	exit 1
else
	FF_ARCH=$1
	echo "-->specify architecture : $FF_ARCH"
fi

LOCAL_PATH=`pwd`
CROSS_COMPILE=
ANDROID_PLATFORM=android-15
BUILD_PATH=$LOCAL_PATH/build
PLATFORM=
PREBUILT=
PREFIX=
FLAGS=
HOST=

function build ()
{
rm -rf ${PREFIX}
cd fdk
set -e
chmod +x configure *.sh
./configure $FLAGS \
--enable-static \
--target=android \
--prefix=${PREFIX}
make clean
make -j4
make install
cd -
}
function build_armv5 ()
{
    CFLAGS=""
	AAC_ARCH=arm
    CROSS_COMPILE=arm-linux-androideabi
	PLATFORM=$NDKROOT/platforms/${ANDROID_PLATFORM}/arch-${AAC_ARCH}
	PREBUILT=$NDKROOT/toolchains/${CROSS_COMPILE}-4.9/prebuilt/linux-x86_64/bin/${CROSS_COMPILE}
    export CPPFLAGS="$CFLAGS"
    export CFLAGS="$CFLAGS"
    export CXXFLAGS="$CFLAGS"
    export CXX="${PREBUILT}g++ --sysroot=${PLATFORM}"
    export LDFLAGS="$LDFLAGS"
    export CC="${PREBUILT}gcc --sysroot=${PLATFORM}"
   export NM="${PREBUILT}nm"
    export STRIP="${PREBUILT}strip"
    export RANLIB="${PREBUILT}ranlib"
    export AR="${PREBUILT}ar"
	HOST=arm-linux-androideabi
	PREFIX=$BUILD_PATH/armeabi/fdk
	FLAGS="--host=$HOST"
	build
}
function build_armv7a ()
{
#    CFLAGS=" -DANDROID -funwind-tables -fstack-protector -fno-short-enums -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__  -Os -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64  -Wa,--noexecstack -MMD -MP "
#	LDFLAGS="-Wl,-Bsymbolic"
    CFLAGS=""
	LDFLAGS=""
	AAC_ARCH=arm
    CROSS_COMPILE=arm-linux-androideabi
	PLATFORM=$NDKROOT/platforms/${ANDROID_PLATFORM}/arch-${AAC_ARCH}
	PREBUILT=$NDKROOT/toolchains/${CROSS_COMPILE}-4.9/prebuilt/linux-x86_64/bin/${CROSS_COMPILE}-
    export CPPFLAGS="$CFLAGS"
    export CFLAGS="$CFLAGS"
    export CXXFLAGS="$CFLAGS"
    export CXX="${PREBUILT}g++ --sysroot=${PLATFORM}"
    export LDFLAGS="$LDFLAGS"
    export CC="${PREBUILT}gcc --sysroot=${PLATFORM}"
    export NM="${PREBUILT}nm"
    export STRIP="${PREBUILT}strip"
    export RANLIB="${PREBUILT}ranlib"
    export AR="${PREBUILT}ar"
	PREFIX=$BUILD_PATH/armeabi-v7a/fdk
	HOST=arm-linux-androideabi
	FLAGS="--host=$HOST"
	build
}
 function build_arm64-v8a ()
{
    CFLAGS=""
	AAC_ARCH=arm64
	ANDROID_PLATFORM=android-21
    CROSS_COMPILE=aarch64-linux-androideabi
	PLATFORM=$NDKROOT/platforms/${ANDROID_PLATFORM}/arch-${AAC_ARCH}
	PREBUILT=$NDKROOT/toolchains/${CROSS_COMPILE}-4.9/prebuilt/linux-x86_64/bin/${CROSS_COMPILE}-
    export CPPFLAGS="$CFLAGS"
    export CFLAGS="$CFLAGS"
    export CXXFLAGS="$CFLAGS"
    export CXX="${PREBUILT}g++ --sysroot=${PLATFORM}"
    export LDFLAGS="$LDFLAGS"
    export CC="${PREBUILT}gcc --sysroot=${PLATFORM}"
    export NM="${PREBUILT}nm"
    export STRIP="${PREBUILT}strip"
    export RANLIB="${PREBUILT}ranlib"
    export AR="${PREBUILT}ar"
	PREFIX=$BUILD_PATH/arm64-v8a/fdk
	HOST=aarch64-linux
	FLAGS="--host=$HOST"
	build
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
    CFLAGS=""
	AAC_ARCH=x86
	ANDROID_PLATFORM=android-15
    CROSS_COMPILE=i686-linux-androideabi
	PLATFORM=$NDKROOT/platforms/${ANDROID_PLATFORM}/arch-${AAC_ARCH}
	PREBUILT=$NDKROOT/toolchains/${CROSS_COMPILE}-4.9/prebuilt/linux-x86_64/bin/${CROSS_COMPILE}-
    export CPPFLAGS="$CFLAGS"
    export CFLAGS="$CFLAGS"
    export CXXFLAGS="$CFLAGS"
    export CXX="${PREBUILT}g++ --sysroot=${PLATFORM}"
    export LDFLAGS="$LDFLAGS"
    export CC="${PREBUILT}gcc --sysroot=${PLATFORM}"
    export NM="${PREBUILT}nm"
    export STRIP="${PREBUILT}strip"
    export RANLIB="${PREBUILT}ranlib"
    export AR="${PREBUILT}ar"
	PREFIX=$BUILD_PATH/x86/fdk
	HOST=i686-linux
	FLAGS="--host=$HOST"
	build
}
 function build_x86_64 ()
{
    CFLAGS=""
	AAC_ARCH=x86_64
	ANDROID_PLATFORM=android-21
    CROSS_COMPILE=x86_64-linux-androideabi
	PLATFORM=$NDKROOT/platforms/${ANDROID_PLATFORM}/arch-${AAC_ARCH}
	PREBUILT=$NDKROOT/toolchains/${CROSS_COMPILE}-4.9/prebuilt/linux-x86_64/bin/${CROSS_COMPILE}-
    export CPPFLAGS="$CFLAGS"
    export CFLAGS="$CFLAGS"
    export CXXFLAGS="$CFLAGS"
    export CXX="${PREBUILT}g++ --sysroot=${PLATFORM}"
    export LDFLAGS="$LDFLAGS"
    export CC="${PREBUILT}gcc --sysroot=${PLATFORM}"
    export NM="${PREBUILT}nm"
    export STRIP="${PREBUILT}strip"
    export RANLIB="${PREBUILT}ranlib"
    export AR="${PREBUILT}ar"
	PREFIX=$BUILD_PATH/x86_64/fdk
	HOST=x86_64-linux
	FLAGS="--host=$HOST"
	build
}
 function build_mips ()
{
    CFLAGS=""
	AAC_ARCH=mips
	ANDROID_PLATFORM=android-15
    CROSS_COMPILE=mipsel-linux-androideabi
	PLATFORM=$NDKROOT/platforms/${ANDROID_PLATFORM}/arch-${AAC_ARCH}
	PREBUILT=$NDKROOT/toolchains/${CROSS_COMPILE}-4.9/prebuilt/linux-x86_64/bin/${CROSS_COMPILE}-
    export CPPFLAGS="$CFLAGS"
    export CFLAGS="$CFLAGS"
    export CXXFLAGS="$CFLAGS"
    export CXX="${PREBUILT}g++ --sysroot=${PLATFORM}"
    export LDFLAGS="$LDFLAGS"
    export CC="${PREBUILT}gcc --sysroot=${PLATFORM}"
    export NM="${PREBUILT}nm"
    export STRIP="${PREBUILT}strip"
    export RANLIB="${PREBUILT}ranlib"
    export AR="${PREBUILT}ar"
	PREFIX=$BUILD_PATH/mips/fdk
	HOST=mipsel-linux
	FLAGS="--host=$HOST"
	build
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
