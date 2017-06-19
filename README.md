# VLM analyzer
The program is a analyzer for VLM(Victory by limited moves) problems.

This repository provides the following interfaces:
* CUI interface located in "cui" directory
* CUI batch interface located in "cui_batch" directory

## Requirements
* git lfs 1.5.6 or newer
* C++ compiler
   * g++ 5.4.0 or newer
   * clang 3.8 or newer
* CMake 3.5.1 or newer
* boost 1.63 or newer

# Optional Requirements
* ccache 3.3.4 or newer if you'd like to compile faster
* Google Test 1.8.0 or newer if you'd like to compile test program
* python texttable module if you summerize cui_batch result logs by cui_batch/summarize_result.py

## Environment variables
Some shell scripts and CMake require the following environment variables:
* BOOST_DIR: the top directory of the Boost Libraries
* GTEST_DIR: the top directory of the Google Test
* MAKE_JOB_NUMBER(optional): the number of jobs to compile C++ files simultaneously

## How to build
You can build VLM analyzer as follows:

* Pull REAL Core library at the vlm_analyzer directory
```shell-session
$ git clone https://github.com/starpentagon/realcore.git
```
* Install packages if you haven't installed CMake and boost yet.
```shell-session
$ sudo apt install cmake
$ sudo apt install libboost-all-dev
```
* Build the VLM analyzer
```shell-session
$ cd cui
$ ./build.sh
```

Now you can run VLM analyzer
```shell-session
$ ./build/vlm_analyzer
```

Also I posted my blog(http://quinstella.net/vlm-analyzer-install-ubuntu/), please find it if you can read Japanese.
