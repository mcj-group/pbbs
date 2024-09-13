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
#include <chrono>
#include <thread>
#include <string>
#include "parlay/primitives.h"
#include "common/graph.h"
#include "MIS.h"
#include "BucketStructs.h"
#include "MultiBucketQueue.h"
#include "MultiQueue.h"

// **************************************************************
//    MAXIMAL INDEPENDENT SET
// **************************************************************
using namespace std;

struct stat {
  uint64_t iter = 0;
  uint64_t dead = 0;
  uint64_t reinsert = 0;
};

template <typename MQ_Type>
void threadTask(Graph const &G, atomic<uint8_t> *vertexFlags,
                MQ_Type &wl, stat *stats)
{
  uint64_t iters = 0, deads = 0, reinserts = 0;
  uint32_t vertex;
  wl.initTID();
  while (true) {
    iters++;
    auto item = wl.pop();
    if (item) std::tie(std::ignore, vertex) = item.get();
    else break;

    // For each vertex:
    //   vertexFlags = 0, included
    //   vertexFlags = 1, excluded
    
    // marked dead
    if (vertexFlags[vertex].load(memory_order_seq_cst) == 1) {
      deads++;
      continue;
    }

    bool proceed = true;
    const vertexId* begin = &G[vertex].Neighbors[0];
    const vertexId* end = &G[vertex].Neighbors[G[vertex].degree];
    for (const vertexId* it = begin; it != end; it++) {
      vertexId ngh = *it;
      // check for unprocessed predecessors
      if (vertexFlags[ngh].load(memory_order_seq_cst) == 0 && ngh < vertex) {
        proceed = false;
        if (!vertexFlags[vertex].load(memory_order_seq_cst)) {
          wl.push(vertex, vertex);
          reinserts++;
        }
        break;
      }
    }

    // set neighbors to excluded
    if (proceed) {
      for (const vertexId* it = begin; proceed && it != end; it++) {
        vertexId ngh = *it;
        if (!vertexFlags[ngh].load(memory_order_seq_cst)) {
          vertexFlags[ngh].store(1, memory_order_seq_cst);
        }
      }
    }
  }
  stats->iter = iters;
  stats->dead = deads;
  stats->reinsert = reinserts;
}

template <typename MQ_Type>
void spawnTasks(Graph const &G, MQ_Type &wl, atomic<uint8_t> *vertexFlags, int threadNum)
{
  for (size_t i = 0; i < G.n; i++) {
    wl.push(i, i);
  }

  stat stats[threadNum];
  for (int i = 0; i < threadNum;i++) {
    stats[i].iter = 0;
    stats[i].dead = 0;
    stats[i].reinsert = 0;
  }

  std::cout << "starting...\n";
  auto start = chrono::high_resolution_clock::now();

  vector<thread*> workers;
  cpu_set_t cpuset;
  for (int i = 1; i < threadNum; i++) {
    CPU_ZERO(&cpuset);
    uint64_t coreID = i;
    CPU_SET(coreID, &cpuset);
    thread *newThread = new thread(
      threadTask<MQ_Type>, ref(G), ref(vertexFlags), 
      ref(wl), &stats[i]
    );
    int rc = pthread_setaffinity_np(newThread->native_handle(),
                                    sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
      cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }
    workers.push_back(newThread);
  }

  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);
  sched_setaffinity(0, sizeof(cpuset), &cpuset);
  threadTask<MQ_Type>(G, vertexFlags, wl, &stats[0]);
  for (thread*& worker : workers) {
    worker->join();
    delete worker;
  }

  auto end = chrono::high_resolution_clock::now();
  auto ms = chrono::duration_cast<chrono::milliseconds>(end-start).count();
  wl.stat();

  uint64_t totalIter = 0;
  uint64_t totalDeads = 0;
  uint64_t totalReInserts = 0;
  for (int i = 0; i < threadNum; i++) {
    totalIter += stats[i].iter;
    totalDeads += stats[i].dead;
    totalReInserts += stats[i].reinsert;
  }

  cout << "runtime_ms " << ms << "\n";
  cout << "totalIter = " << totalIter << endl;
  cout << "totalDeads = " << totalDeads << endl;
  cout << "totalReInserts = " << totalReInserts << endl;
}

template<bool usePrefetch>
parlay::sequence<char> run(
  Graph const &G, char* qType, int threadNum, int queueNum,
  int batchSizePop, int batchSizePush, int delta, int bucketNum, int stickiness)
{
  cout << "\nRunning MQ based MIS" << endl;
  size_t n = G.n;
  cout << "Graph Size: "<< n << endl;
  parlay::sequence<char> Flags(n, (char) 0);
  atomic<uint8_t> *vertexFlags = new atomic<uint8_t>[n]();

  cout << "type: "<< qType << endl;
  cout << "threadNum: "<< threadNum << endl;
  cout << "queueNum: "<< queueNum << endl;
  cout << "batchSizePop: "<< batchSizePop << endl;
  cout << "batchSizePush: "<< batchSizePush << endl;
  cout << "stickiness: "<< stickiness << endl;
  cout << "prefetch: " << usePrefetch << "\n";

  std::function<void(uint32_t)> prefetcher = [&] (uint32_t v) -> void {
    __builtin_prefetch(&vertexFlags[v], 0, 3);
  };

  std::string qTypeStr(qType);
  if (qTypeStr == "MQBucket") {
    cout << "delta: "<< delta << endl;
    cout << "bucketNum: "<< bucketNum << endl;
    std::function<mbq::BucketID(uint32_t)> getBucketID = [&] (uint32_t v) -> mbq::BucketID {
      return ((mbq::BucketID)v >> delta);
    };
    using MQ_Bucket = mbq::MultiBucketQueue<
      decltype(getBucketID), decltype(prefetcher), greater<mbq::BucketID>, 
      uint32_t, uint32_t, usePrefetch>;
    MQ_Bucket wl(
      getBucketID, prefetcher, queueNum, threadNum, delta, bucketNum,
      batchSizePop, batchSizePush, mbq::increasing, stickiness);
    spawnTasks<MQ_Bucket>(G, wl, vertexFlags, threadNum);  
    
  } else if (qTypeStr == "MQ"){
    using PQElement = std::tuple<uint32_t, uint32_t>;
    using MQ = mbq::MultiQueue<
      decltype(prefetcher), std::greater<PQElement>, 
      uint32_t, uint32_t, usePrefetch>;
    MQ wl(prefetcher, queueNum, threadNum, batchSizePop, batchSizePush, stickiness);
    spawnTasks<MQ>(G, wl, vertexFlags, threadNum);  

  } else {
    cout << qTypeStr << ": no matching type!\n";
    exit(0);
  }

  for (int i = 0; i < n; i++) {
    if (vertexFlags[i] == 0)
      Flags[i] = 1;
  }

  delete [] vertexFlags;

  return Flags;
}

parlay::sequence<char> maximalIndependentSet(
  Graph const &G, char* qType, int threadNum, int queueNum,
  int batchSizePop, int batchSizePush, int delta, int bucketNum,
  int stickiness, bool usePrefetch)
{
  if (usePrefetch) 
    return run<true>(
      G, qType, threadNum, queueNum, batchSizePop, batchSizePush,
      delta, bucketNum, stickiness);
  else
    return run<false>(
      G, qType, threadNum, queueNum, batchSizePop, batchSizePush,
      delta, bucketNum, stickiness);
}
