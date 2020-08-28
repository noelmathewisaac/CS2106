#!/bin/bash

function check_files {
    for i do
        if [ ! -f $i ]; then
            return 1
        fi
    done
}

function check_compile {
    x=1
    cd $1
        make &> /dev/null
        x=$?
    cd ../
    [[ $x -ne 0 ]]
}

function run_tests {
    tests=("sample" "small_test" "big_test")
    cd $1
    make &> /dev/null
    for test in "${tests[@]}"; do
        if [[ $1 == "ex1" ]]
        then
            ./ex1 < $test.in | diff $test.out - &> /dev/null
            res=$?
        else
            ./ex2 $test.in | diff $test.out - &> /dev/null
            res=$?
        fi

        if [[ res -ne 0 ]]; then
            echo "Test $test.in failed"
            break
        fi
    done
    cd ../
}

function transfer_files {
    files=(
        "Makefile"
        "node.h"
        "sample.in"
        "sample.out"
        "small_test.in"
        "small_test.out"
        "big_test.in"
        "big_test.out"
    )

    if [[ $2 == "ex1" ]]
    then
        files+=("ex1.c")
    else
        files+=("functions.h" "functions.c")
    fi

    for file in "${files[@]}"; do
        cp $1/$2/$file $2
    done
}

echo "Checking zip file...."

if [ "$#" -ne 1 ]; then
    echo "You should provide your zip file as a parameter and nothing else!"
    exit 1
fi

if ! [[ $1 =~ ^E[0-9]{7}.zip$ ]]; then
    echo "zip file is wrongly named: it should be <NUSNET ID>.zip"
    exit 1
fi

echo "Unzipping file: $1"
if [ ! -f $1 ]; then
    echo "File $1 does not exist."
    exit 1
fi

tmp_folder="test_grading_aeN332Hp"
rm -rf $tmp_folder
mkdir $tmp_folder

cp $1 $tmp_folder

cd $tmp_folder
unzip $1 > /dev/null

echo "Transferring necessary skeleton files"
sk="skeleton"
mkdir $sk
cd $sk
wget "https://www.comp.nus.edu.sg/~ccris/cs2106_ay2021s1/lab1.tar.gz" > /dev/null
tar -zxvf lab1.tar.gz > /dev/null
cd ../

exercises=(ex1 ex2 ex4 ex5)
ex1=("ex1/node.c")
ex2=("ex2/node.c" "ex2/function_pointers.c" "ex2/function_pointers.h" "ex2/ex2.c")
ex4=("ex4/check_system.sh")
ex5=("ex5/check_syscalls.sh")

for ex in "${exercises[@]}"; do
    declare -n files=$ex
    if [ ! -d $ex ]
    then
        echo "$ex: Failed - $ex folder missing"
        continue
    fi

    if ! check_files "${files[@]}"
    then
        echo "$ex: Failed - files missing"
        continue
    fi

    if [[ $ex == @('ex1'|'ex2') ]]
    then
        transfer_files $sk $ex

        if check_compile $ex
        then
            echo "$ex: Failed - does not compile"
            continue
        fi

        test_status="$(run_tests $ex)"
        if ! [ -z "$test_status" ]; then
            echo "$ex: Failed - $test_status"
            continue
        fi
    fi

    echo "$ex: Success"
done

cd ../
rm -rf $tmp_folder
