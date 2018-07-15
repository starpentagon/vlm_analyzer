#!/bin/bash

./build.sh

input_file=VLM_u.csv
result_file=result.csv
thread_num=3

./build/vlm_analyzer_batch --problem-db ${input_file} --dual --depth 7 --thread ${thread_num} > ${result_file}
./sort_and_add_header.sh ${result_file}

./summrize_result.py ${result_file}

