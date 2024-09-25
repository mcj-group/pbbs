#include "common/graph.h"

using vertexId = uint;
using edgeId = uint;
using Graph = graph<vertexId,edgeId>;


parlay::sequence<char> maximalIndependentSet(
  Graph const &G, char* qType, int threadNum, int queueNum, int batchSizePop,
  int batchSizePush, int delta, int bucketNum, int stickiness, bool usePrefetch
);

