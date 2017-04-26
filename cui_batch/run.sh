#!/bin/bash

./build.sh

result_file=result.csv

./build/vlm_analyzer_batch --problem-db VLM_u_20170423.csv --dual --depth 5 --thread 4 > ${result_file}
./sort_and_add_header.sh ${result_file}
