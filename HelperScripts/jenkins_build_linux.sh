#!/bin/bash -xe
cd $WORKSPACE/LarkX
git submodule init
git submodule update
mkdir $WORKSPACE/build
cd $WORKSPACE/build
export BITSHARES_ROOT=$WORKSPACE
. ../LarkX/setenv.sh
cmake -DINCLUDE_QT_WALLET=OFF \
      -DCMAKE_TOOLCHAIN_FILE=$WORKSPACE/toolchain.invictus/toolchain.invictus.cmake \
      ../LarkX
make -j8
