# VLM analyzer
The program is a problem analyzer for VLM(Victory by limited moves).

This repository provides the following interfaces:
* CUI interface located in "cui" directory
* CUI batch interface located in "cui_batch" directory

## Requirements
* git lfs 1.5.6 or newer
* C++ compiler
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
* MAKE_JOB_NUMBER: the number of jobs to compile C++ files simultaneously

