// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <algorithm>
#include "parlay/parallel.h"
#include "common/time_loop.h"
#include "common/graph.h"
#include "common/IO.h"
#include "common/graphIO.h"
#include "common/parse_command_line.h"
#include "MIS.h"
using namespace std;
using namespace benchIO;

void timeMIS(
  Graph const &G, int rounds, char* outFile, char* qType, int threadNum, int queueNum,
  int batchSizePop, int batchSizePush, int delta, int bucketNum, int stickiness, bool usePrefetch) {
  parlay::sequence<char> flags = maximalIndependentSet(
    G, qType, threadNum, queueNum, batchSizePop, batchSizePush, delta, bucketNum, stickiness, usePrefetch);
  time_loop(rounds, 1.0,
	    [&] () {flags.clear();},
	    [&] () {flags = maximalIndependentSet(
                G, qType, threadNum, queueNum, batchSizePop, batchSizePush,
                delta, bucketNum, stickiness, usePrefetch);},
	    [&] () {});
  cout << endl;
  
  auto F = parlay::tabulate(G.n, [&] (size_t i) -> int {return flags[i];});
  writeIntSeqToFile(F, outFile);
}


int main(int argc, char* argv[]) {
  commandLine P(argc, argv, "[-o <outFile>] [-r <rounds>] <inFile>");
  cout << "running MIS\n";
  char* iFile = P.getArgument(0);
  char* oFile = P.getOptionValue("-o");
  char* qType = P.getOptionValue("-type");
  int rounds = P.getOptionIntValue("-r",1);
  int threadNum = P.getOptionIntValue("-threads", 1);
  int queueNum = P.getOptionIntValue("-queues", 2);
  int batchSizePop = P.getOptionIntValue("-batch1", 1);
  int batchSizePush = P.getOptionIntValue("-batch2", 1);
  int delta = P.getOptionIntValue("-delta", 1);
  int bucketNum = P.getOptionLongValue("-buckets", 64);
  int stickiness = P.getOptionLongValue("-stick", 1);
  bool usePrefetch = P.getOptionValue("-prefetch");
  Graph G = readGraphFromFile<vertexId,edgeId>(iFile);
  timeMIS(G, rounds, oFile, qType, threadNum, queueNum, batchSizePop,
          batchSizePush, delta, bucketNum, stickiness, usePrefetch);
}
