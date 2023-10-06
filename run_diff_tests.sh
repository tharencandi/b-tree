#!/bin/bash

echo "E2E TESTING FOR BTREE"
echo
count=0
passed=0

for file in diff_tests/*.out; do
    name=$(basename $file .out)
    echo "-----------------------------------"
    echo "DIF TESTING: $name"

    diff diff_tests/$name.out diff_tests/$name.correct

    count=$((count+1))
done

echo 
echo "$count TESTS COMPLETED"