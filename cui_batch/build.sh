#!/bin/bash
compiler=g++

if [ ! -d build ]; then
	mkdir build
fi

pushd build

cmake -DCMAKE_CXX_COMPILER=${compiler} ..
make -j ${MAKE_JOB_NUMBER}

popd

