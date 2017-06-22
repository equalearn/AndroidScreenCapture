#!/bin/sh

# This script runs a configure script with the Android NDK toolchain
# You may need to adjust the COMPILE_TARGET and ANDROID_API variables
# depending on your requirements.
#
# Call this script in a directory with a valid configure script.
# Example: PREFIX=${PWD}/bin android-configure.sh 

# Set the ANDROID_NDK variable to the root
# Example: export ANDROID_NDK=${HOME}/.local/android-ndk-r9b

if [ -z ${ANDROID_NDK} ]; then
  echo "Please set ANDROID_NDK environment variable to the root directory of the Android NDK"
  exit 1
fi

# This is just an empty directory where I want the built objects to be installed
if [ -z ${PREFIX} ]; then
  echo "Please set PREFIX environment variable to the output directory"
  exit 1
fi

export COMPILE_TARGET=arm-linux-androideabi
export ANDROID_API=23
#export ANDROID_API=19
export GCC_VER="4.9"


export ANDROID_PREFIX=${ANDROID_NDK}/toolchains/${COMPILE_TARGET}-${GCC_VER}/prebuilt/linux-x86_64
export SYSROOT=${ANDROID_NDK}/platforms/android-${ANDROID_API}/arch-arm


export TOOLCHAIN_PATH=${ANDROID_PREFIX}/bin

# These are the toolchain utilities
export CPP=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-cpp
export AR=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-ar
export AS=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-as
export NM=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-nm
export CC=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-gcc
export CXX=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-g++
export LD=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-ld
export RANLIB=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-ranlib
export STRIP=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-strip
export OBJDUMP=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-objdump
export OBJCOPY=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-objcopy
export ADDR2LINE=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-addr2line
export READELF=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-readelf
export SIZE=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-size
export STRINGS=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-strings
export ELFEDIT=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-elfedit
export GCOV=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-gcov
export GDB=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-gdb
export GPROF=${TOOLCHAIN_PATH}/${COMPILE_TARGET}-gprof

# Don't mix up .pc files from your host and build target
export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig

export CFLAGS="${CFLAGS} --sysroot=${SYSROOT} -I${SYSROOT}/usr/include -I${ANDROID_PREFIX}/include"
#export CPPFLAGS="${CFLAGS} -I${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/include -I${ANDROID_NDK}/sysroot/usr/include"
export CPPFLAGS="${CFLAGS}" 
#export LDFLAGS="${LDFLAGS} --sysroot=${SYSROOT} -pie -fPIE -L${SYSROOT}/usr/lib -L${ANDROID_PREFIX}/lib" 
#export LDFLAGS="${LDFLAGS} --stl=stlport -L${SYSROOT}/usr/lib -L${ANDROID_PREFIX}/lib"
export LDFLAGS="${LDFLAGS} -L${SYSROOT}/usr/lib -L${ANDROID_PREFIX}/lib"

./configure --host=${COMPILE_TARGET}  --prefix=${PREFIX} "$@"
