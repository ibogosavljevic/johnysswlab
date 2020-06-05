#!/bin/bash

# Modify this variable to correspond to FlameGraphs on your computer
PATH_TO_FLAMEGRAPHS=/home/ivica/Projects/FlameGraph/

if [ ! -d "$PATH_TO_FLAMEGRAPHS" ]
then
    echo "Modify PATH_TO_FLAMEGRAPHS variable in this script to corresponf to FlameGraphs directory on your computer"
    exit 1
fi

function show_help {
    echo "$0" records the execution of your program and displays it as beautifull flame flame graphs
    echo "Internally it uses perf profiler and flamegraphs"
    echo "You can get flamegraphs with git clone https://github.com/brendangregg/FlameGraph"
    echo
    echo "To run it, type"
    echo "    $0 -perfopt1 perfval -perfopt2 perfval ... ./my_program arg1 arg2 ..."
    echo "-perfopt* are passed to pass record"
    echo    
    echo "Examples:"
    echo "    $0 ./my_program"
    echo "        Executes ./my_program with default perf parameters"
    echo "    $0 -F 50 ./my_program arg1"
    echo "        Passes option -F 50 to perf record and executes your program with arg1. This means perf will take 50 samples per second"
    echo "Output is written to .svg file. You can use a web browser to display it."
}

if [ $# -eq 0 ]; then
    echo "No arguments provided"
    show_help
    exit 1
fi


PERF_PARAMS=""

shopt -s extglob

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -h|--help)
    show_help
    exit 0
    ;;
    -*)
    PERF_PARAMS="${PERF_PARAMS} $1 $2"
    shift
    shift
    ;;
    *)
    ARGS="$@"
    break
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

PATH=$PATH_TO_FLAMEGRAPHS:$PATH

RECORD_COMMAND="perf record ${PERF_PARAMS} --call-graph dwarf ${ARGS}"
CONVERT_COMMAND="perf script | stackcollapse-perf.pl | stackcollapse-recursive.pl | flamegraph.pl > out.svg"

echo "Running " $RECORD_COMMAND 
eval $RECORD_COMMAND
echo "Running " $CONVERT_COMMAND
eval $CONVERT_COMMAND
echo "Output captured to file out.svg"