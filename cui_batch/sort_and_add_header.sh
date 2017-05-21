#!/bin/bash

if [ "$1" = "" ]; then
  base=`basename $0`
  echo "Usage: ./${base} (csv_file)"
  exit
fi

csv_file=$1
tmp_file=${csv_file}.tmp

./build/vlm_analyzer_batch --header > ${tmp_file}
sort ${csv_file} >> ${tmp_file}

rm ${csv_file}
mv ${tmp_file} ${csv_file}
