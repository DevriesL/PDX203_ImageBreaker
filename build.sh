#!/bin/bash

mkdir out

GCC_PATH=~/Workspace/Kernel/Toolchain/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin
CLANG_PATH=~/Workspace/Kernel/Toolchain/clang/clang-r377782d/bin

BUILD_CROSS_COMPILE=${GCC_PATH}/aarch64-linux-gnu-
CLANG_TRIPLE=aarch64-linux-gnu-

DEFCONFIG=vendor/kona-perf_defconfig

export KBUILD_DIFFCONFIG=imagebreaker_diffconfig
export PATH=${CLANG_PATH}:${PATH}

make -j56 -C $(pwd) O=$(pwd)/out ARCH=arm64 \
    CC=clang CLANG_TRIPLE=aarch64-linux-gnu- \
    CROSS_COMPILE=$BUILD_CROSS_COMPILE $DEFCONFIG

make -j56 -C $(pwd) O=$(pwd)/out ARCH=arm64 \
    CC=clang CLANG_TRIPLE=aarch64-linux-gnu- \
    CROSS_COMPILE=$BUILD_CROSS_COMPILE 2>&1 | tee build.txt
 
cp out/arch/arm64/boot/Image $(pwd)/boot.img-zImage
