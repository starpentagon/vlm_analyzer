#!/bin/bash

./build.sh

result_file=result.csv

./build/vlm_analyzer_batch --problem-db VLM_u_20170617.csv --dual --depth 7 --thread 3 > ${result_file}
./sort_and_add_header.sh ${result_file}

./summrize_result.py ${result_file}

