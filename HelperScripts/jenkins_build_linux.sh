#!/bin/bash -xe
cd $WORKSPACE/PTS
git submodule init
git submodule update
mkdir $WORKSPACE/build
cd $WORKSPACE/build
export BITSHARES_ROOT=$WORKSPACE
. ../PTS/setenv.sh
cmake -DINCLUDE_QT_WALLET=OFF \
      -DCMAKE_TOOLCHAIN_FILE=$WORKSPACE/toolchain.invictus/toolchain.invictus.cmake \
      ../PTS
make -j8
