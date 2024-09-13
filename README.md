# pbbsbench

This is a minimalistic version of the PBBS repo that only contains
necessary files for MIS.

`maximalIndependentSet/mqMIS` contains the code for MultiQueue-based
implementations of greedy maximal independent set.

These implementations rely on the MultiQueue files from cps repo.

# Prerequisite
The executables are built with Clang 16 with OpenCilk 2.1
and the MQ versions require boost >= 1.58.0.

# Build & Run
```
# build
make

# run the incremental MIS
./runincremental.sh

# do a single run of the incremental MIS with R rounds and N threads
CILK_NWORKERS=<N> ./maximalIndependentSet/incrementalMIS/MIS -o output.log -r <R> input_graph


# run the MQ-based MIS
./runmq.sh

# do a single run of the MBQ MIS
./maximalIndependentSet/mqMIS/MIS -o output.log -r <R> -delta <D> -type MQBucket -threads <T> -queues <Q> -buckets <B> -batch1 <b1> -batch2 <b2> <-prefetch> -stick <s> input_graph
```
