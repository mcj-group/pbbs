#!/bin/bash

input_dir=../../inputs_pbbs/symm-ag
exec_dir=./maximalIndependentSet/incrementalMIS

graphs=(USA LJ TW HYPERH)


for i in {0..3}
do
	echo "Running MIS on ${graphs[i]}"
	echo "Results" > results_mis_48_${graphs[i]}_incremental.log
	CILK_NWORKERS=48 ${exec_dir}/MIS -o ./incremental_${graphs[i]}.log -r 10 ${input_dir}/${graphs[i]}.sym.ag >> results_mis_48_${graphs[i]}_incremental.log
done

for i in {0..3}
do
        echo "Running MIS on ${graphs[i]}"
        echo "Results" > results_mis_1_${graphs[i]}_incremental.log
        CILK_NWORKERS=1 ${exec_dir}/MIS -o ./incremental_${graphs[i]}.log -r 10 ${input_dir}/${graphs[i]}.sym.ag >> results_mis_1_${graphs[i]}_incremental.log
done

