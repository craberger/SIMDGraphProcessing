#!/bin/bash

TIMEOUT=1800

for i in `seq 1 ${1}`; do
  if [ -f $1/attributes.txt ]; then
    echo "Currently no queries with attributes"
  else
     ln -f -s "${2}/glab_undirected/data.txt" input.txt

     lb create --overwrite benchmarking
     lb addblock -f add_edges.lb benchmarking
     lb exec -f load_data.lb benchmarking

     # for app in "cliqueCounting" "cycleCounting" "lollipopCounting"; do
     for app in "triangleCounting" "cliqueCounting"; do #"similarity" "symbiosity"; do
        echo "----- ${app} -----"
        #./launch_server.sh
        time timeout $TIMEOUT lb exec -f ${app}.lb benchmarking
        #unit --sequential --threads $3 --time --test ${app}.lb
        #./stop_server.sh
     done

     lb delete benchmarking
  fi
done
