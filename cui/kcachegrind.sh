#!/bin/bash

rm -rf build

./build.sh

date=`date +%y%m%d`
log_name=kcache_${date}_$$.log

valgrind --tool=callgrind --log-file=${log_name} ./build/vlm_analyzer -s hhgihiihigffiihffjifgfijgjjfhjjgjiefeeekfkfegehkhekeiekiikkjjehdkfhlilidljle

callgrind_pid=`head -n 1 ${log_name} | sed -e "s/^==//g" | sed -e "s/==.*//g"`
kcachegrind callgrind.out.${callgrind_pid} &
rm ${log_name}
