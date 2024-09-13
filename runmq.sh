#!/bin/bash

input_dir=../../inputs_pbbs/symm-ag
exec_dir=./maximalIndependentSet/mqMIS

graphs=(USA LJ TW HYPERH)

deltas=(16 11 21 21)
batches=(256 128 256 256)

# MQBucket
for i in {0..3}
do
	echo "Results" > results_mis_48_${graphs[i]}_MQBucket.log
	${exec_dir}/MIS -o ./${graphs[i]}_MQBucket.log -r 10 -delta ${deltas[i]} -type MQBucket -threads 48 -queues 192 -buckets 64 -batch1 ${batches[i]} -batch2 ${batches[i]} -prefetch ${input_dir}/${graphs[i]}.sym.ag >> results_mis_48_${graphs[i]}_MQBucket.log
done

for i in {0..3}
do
        echo "Results" > results_mis_1_${graphs[i]}_MQBucket.log
        ${exec_dir}/MIS -o ./${graphs[i]}_MQBucket.log -r 10 -delta ${deltas[i]} -type MQBucket -threads 1 -queues 4 -buckets 64 -batch1 ${batches[i]} -batch2 ${batches[i]} -prefetch ${input_dir}/${graphs[i]}.sym.ag >> results_mis_1_${graphs[i]}_MQBucket.log
done

# MQ
for i in {0..3}
do      
        echo "Results" > results_mis_48_${graphs[i]}_MQ.log
        ${exec_dir}/MIS -o ./${graphs[i]}_MQ.log -r 10 -type MQ -threads 48 -queues 192 -buckets 64 -batch1 ${batches[i]} -batch2 ${batches[i]} -prefetch ${input_dir}/${graphs[i]}.sym.ag >> results_mis_48_${graphs[i]}_MQ.log
done    

for i in {0..3}
do
        echo "Results" > results_mis_1_${graphs[i]}_MQ.log
        ${exec_dir}/MIS -o ./${graphs[i]}_MQ.log -r 10 -type MQ -threads 1 -queues 4 -buckets 64 -batch1 ${batches[i]} -batch2 ${batches[i]} -prefetch ${input_dir}/${graphs[i]}.sym.ag >> results_mis_1_${graphs[i]}_MQ.log
done

# MQPlain
for i in {0..3}
do
        echo "Running MIS MQPlain on ${graphs[i]}"
        echo "Results" > results_mis_48_${graphs[i]}_MQPlain.log
        ${exec_dir}/MIS -o ./${graphs[i]}_MQPlain.log -r 10 -type MQ -threads 48 -queues 192 -batch1 1 -batch2 1 ${input_dir}/${graphs[i]}.sym.ag >> results_mis_48_${graphs[i]}_MQPlain.log
done

for i in {0..3}
do
        echo "Running MIS MQPlain on ${graphs[i]}"
        echo "Results" > results_mis_1_${graphs[i]}_MQPlain.log
        ${exec_dir}/MIS -o ./${graphs[i]}_MQPlain.log -r 10 -type MQ -threads 1 -queues 4 -batch1 1 -batch2 1 ${input_dir}/${graphs[i]}.sym.ag >> results_mis_1_${graphs[i]}_MQPlain.log
done

