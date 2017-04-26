#!/bin/bash
compiler=g++

if [ ! -d build ]; then
	mkdir build
fi

pushd build

cmake -DCMAKE_CXX_COMPILER=${compiler} -DCMAKE_CXX_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg -DCMAKE_SHARED_LINKER_FLAGS=-pg ..
make -j ${MAKE_JOB_NUMBER}

popd

