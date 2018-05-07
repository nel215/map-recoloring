#ifndef MAP_RECOLORING_HPP_
#define MAP_RECOLORING_HPP_
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <set>
#include <queue>
#include <stack>
#include <string>
#include <memory>
#include <utility>

using namespace std;

#ifdef LOCAL
const double ticks_per_sec = 3200000000;
const double timeLimit = 6.0;
#else
const double ticks_per_sec = 2800000000;
const double timeLimit = 9.7;
#endif  // LOCAL
inline double getTime() {
    uint32_t lo, hi;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return (((uint64_t)hi << 32) | lo) / ticks_per_sec;
}

struct Node {
  int region;
  int color;
  int prev;
  int usedColor;
  int recolor;
  int remDegree;
  int payed;
  int remArea;
  Node(int _region, int _color, int _prev, int _usedColor, int _recolor, int _remDegree, int _payed, int _remArea) {
    region = _region;
    color = _color;
    prev = _prev;
    usedColor = _usedColor;
    recolor = _recolor;
    remDegree = _remDegree;
    payed = _payed;
    remArea = _remArea;
  }
  bool operator<(const Node &n)const {
    if (usedColor*recolor == n.usedColor*n.recolor) {
      if (usedColor == n.usedColor) {
        if (recolor == n.recolor) {
          if (remDegree == n.remDegree) {
            if (payed == n.payed) {
              return remArea < n.remArea;
            }
            return payed < n.payed;
          }
          return remDegree < n.remDegree;
        }
        return recolor < n.recolor;
      }
      return usedColor < n.usedColor;
    }
    return usedColor*recolor < n.usedColor*n.recolor;
  }
  bool operator<=(const Node &n)const {
    if (usedColor*recolor == n.usedColor*n.recolor) {
      if (usedColor == n.usedColor) {
        if (recolor == n.recolor) {
          if (remDegree == n.remDegree) {
            if (payed == n.payed) {
              return remArea <= n.remArea;
            }
            return payed < n.payed;
          }
          return remDegree < n.remDegree;
        }
        return recolor < n.recolor;
      }
      return usedColor < n.usedColor;
    }
    return usedColor*recolor < n.usedColor*n.recolor;
  }
  bool operator>(const Node &n)const {
    return n < (*this);
  }
  bool operator>=(const Node &n)const {
    return n <= (*this);
  }
};

// min-max heap
template<class T>
class MinMaxHeap{
  std::vector<T> data;

  inline int getParent(int i) const {
    return (i - 1) >> 1;
  }
  inline int getGrandParent(int i)const {
    return (i - 3) >> 2;
  }
  inline bool hasParent(int i)const {
    return i - 1 >= 0;
  }
  inline bool hasGrandParent(int i)const {
    return i - 3 >= 0;
  }
  inline bool isMinLevel(int i)const {
    bool res = true;
    for (int j = 0; j < i; j = 2 * j + 2) {
      res = !res;
    }
    return res;
  }
  inline bool isChild(int i, int c) {
    return 2 * i + 1 <= c && c <= 2 * i + 2;
  }
  inline bool hasChild(int i) {
    return 2 * i + 1 < data.size();
  }
  // trickleDown
  void trickleDownMin(int i) {
    for (size_t j = i, minIdx; hasChild(j); j = minIdx) {
      minIdx = 2 * j + 1;
      for (int g = 4 * j + 3, last = min(4 * j + 7, data.size()); g < last; g++) {
        if (data[g] < data[minIdx])minIdx = g;
      }
      if (2 * j + 2 < data.size() && data[2 * j + 2] <= data[minIdx])minIdx = 2 * j + 2;
      if (data[minIdx] >= data[j])break;
      std::swap(data[minIdx], data[j]);
      if (isChild(j, minIdx))break;
      if (data[minIdx] > data[getParent(minIdx)]) {
        std::swap(data[minIdx], data[getParent(minIdx)]);
      }
    }
  }
  void trickleDownMax(int i) {
    for (size_t j = i, maxIdx; hasChild(j); j = maxIdx) {
      maxIdx = 2 * j + 1;
      for (int g = 4 * j + 3, last = min(4 * j + 7, data.size()); g < last; g++) {
        if (data[g] > data[maxIdx])maxIdx = g;
      }
      if (2 * j + 2 < data.size() && data[2 * j + 2] >= data[maxIdx])maxIdx = 2 * j + 2;
      if (data[maxIdx] <= data[j])break;
      std::swap(data[maxIdx], data[j]);
      if (isChild(j, maxIdx))break;
      if (data[maxIdx] < data[getParent(maxIdx)]) {
        std::swap(data[maxIdx], data[getParent(maxIdx)]);
      }
    }
  }
  void trickleDown(int i) {
    if (isMinLevel(i)) {
      trickleDownMin(i);
    } else {
      trickleDownMax(i);
    }
  }
  // bubbleUp
  void bubbleUpMin(int i) {
    for (int j = i; hasGrandParent(j) && data[j] < data[getGrandParent(j)]; j = getGrandParent(j)) {
      std::swap(data[j], data[getGrandParent(j)]);
    }
  }
  void bubbleUpMax(int i) {
    for (int j = i; hasGrandParent(j) && data[j] > data[getGrandParent(j)]; j = getGrandParent(j)) {
      std::swap(data[j], data[getGrandParent(j)]);
    }
  }

  void bubbleUp(int i) {
    if (isMinLevel(i)) {
      if (hasParent(i) && data[i] > data[getParent(i)]) {
        std::swap(data[i], data[getParent(i)]);
        bubbleUpMax(getParent(i));
      } else {
        bubbleUpMin(i);
      }
    } else {
      if (hasParent(i) && data[i] < data[getParent(i)]) {
        std::swap(data[i], data[getParent(i)]);
        bubbleUpMin(getParent(i));
      } else {
        bubbleUpMax(i);
      }
    }
  }

 public:
  int size() {
      return data.size();
  }

  bool empty() {
      return data.empty();
  }

  void push(T x) {
    data.push_back(x);
    bubbleUp(data.size() - 1);
  }
  void deleteMin() {
    std::swap(data[0], data[data.size() - 1]);
    data.pop_back();
    trickleDown(0);
  }
  void deleteMax() {
    if (data.size() == 1)return deleteMin();
    int maxIdx = 1;
    if (data.size() >= 3 && data[2] > data[1]) maxIdx = 2;
    std::swap(data[maxIdx], data[data.size() - 1]);
    data.pop_back();
    trickleDown(maxIdx);
  }

  T getMin() const {
    return data[0];
  }

  T getMax() const {
    return data.size() >= 3 ? std::max<T>(data[1], data[2]) : data[data.size() == 2];
  }
};

// rng
class XorShift {
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint32_t w;
  uint64_t max_uint32 = static_cast<uint32_t>(-1);
 public:
  explicit XorShift(int seed) {
    std::srand(seed);
    x = std::rand();
    y = std::rand();
    z = std::rand();
    w = std::rand();
  }
  uint32_t rand() {
    uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
  }
  double uniform() {
    double a = rand();
    return a/(max_uint32+1);
  }
};
XorShift rng(215);

struct Mask {
  vector<uint64_t> data;
  explicit Mask(int H) {
    data.assign(2*H+4, 0);
  }
  bool get(int y, int x) const {
    y++;
    x++;
    return data[2*y+(x>>6)] & (1LL << (x&63));
  }
  void set(int y, int x) {
    y++;
    x++;
    data[2*y+(x>>6)] |= (1LL << (x&63));
  }
  void reset() {
    for (int i=0; i < data.size(); i++) {
      data[i] = 0;
    }
  }
};

const int dy[4] = {-1, 0, 1, 0};
const int dx[4] = {0, -1, 0, 1};

struct Region {
  int color;
  vector<int> link;
  explicit Region(int _color, const vector<int> _link):
    color(_color), link(_link) {}
};

class MapRecoloring {
  int H, W, R, C;
  double startTime;
  vector<int> area;
  vector<vector<int> > graph;
  vector<int> degree;
  vector<vector<int> > cost;
  vector<vector<int> > minRegionColorCost;
  vector<Region> regions;
  bool isin(int y, int x) {
    return 0 <= y && y < H && 0 <= x && x < W;
  }

 public:
  int getRegionScore(int filled, int degree, int diffCost) {
    return (filled*50000 + degree) + diffCost;
  }
  vector<int> beamSearch() {
    int degreeSum = 0;
    for (int i=0; i < R; i++) {
      degreeSum += regions[i].link.size();
    }
    vector<MinMaxHeap<Node> > queues(R+1, MinMaxHeap<Node>());
    vector<Node> history;
    queues[0].push(Node(-1, -1, -1, 0, 0, degreeSum, H*W, H*W));
    int tmp = 0;

    auto bestNode = Node(-1, -1, -1, 32, H*W, degreeSum, H*W, H*W);
    while (getTime()-startTime < timeLimit) {
      for (int q=0; q < R; q++) {
        auto &que = queues[q];
        while (!que.empty() && bestNode < que.getMin()) {
          que.deleteMin();
        }
        if (que.empty()) continue;
        // cerr << q << " " << que.size() << endl;
        tmp++;

        // reconstruct
        auto node = que.getMin();
        int payed = H*W;
        int remDegree = degreeSum;
        vector<int> usedRegion(R, -1);
        int usedColorSet = 0;
        while (node.prev != -1) {
          payed -= cost[node.region][node.color];
          remDegree -= regions[node.region].link.size();
          usedColorSet |= 1 << node.color;
          usedRegion[node.region] = node.color;
          node = history[node.prev];
        }
        node = que.getMin();
        int prev = history.size();
        history.push_back(node);
        que.deleteMin();

        // select region
        double bestScore = 0;
        int bestIdx = 0;
        int minCostSum = 0;
        int remArea = H*W;
        for (int i=0; i < R; i++) {
          if (usedRegion[i] >= 0) {
            remArea -= area[i];
            continue;
          }
          int filled = 0;
          for (auto r : regions[i].link) {
            if (usedRegion[r] == -1) continue;
            filled |= 1 <<  usedRegion[r];
          }
          int minCost = minRegionColorCost[i][filled&((1 << C)-1)];
          minCostSum += minCost;
          filled = __builtin_popcount(filled);
          double score = getRegionScore(filled, degree[i], area[i]-minCost);
          if (bestScore < score) {
            bestScore = score;
            bestIdx = i;
          }
        }
        if (node.usedColor >= bestNode.usedColor && node.recolor + minCostSum >= bestNode.recolor) {
          continue;
        }

        // select color
        auto &region = regions[bestIdx];
        remDegree -= regions[bestIdx].link.size();
        remArea -= area[bestIdx];
        for (int i=0; i < 32; i++) {
          bool sameColor = false;
          for (auto adjRegion : region.link) {
            if (usedRegion[adjRegion] == i) sameColor = true;
          }
          if (sameColor) continue;

          int recolor = node.recolor + cost[bestIdx][i];
          int usedColor = node.usedColor;
          if ((usedColorSet & (1 << i)) == 0) usedColor++;
          auto next = Node(bestIdx, i, prev, usedColor, recolor, remDegree, payed-cost[bestIdx][i], remArea);
          if (next > bestNode) continue;
          auto &nextQue = queues[q+1];
          if (nextQue.size() < 2000) {
            nextQue.push(next);
          } else if (next < nextQue.getMax()) {
            nextQue.deleteMax();
            nextQue.push(next);
          }
          if (i >= C) break;
        }
      }
      // TODO: optimization
      auto &lastQue = queues[R];
      if (!lastQue.empty()) {
        auto node = lastQue.getMin();
        if (bestNode > node) {
          bestNode = node;
          cerr << "improve" << " " << node.usedColor << endl;
        }
      }
      // while (!lastQue.empty()) lastQue.deleteMin();
    }

    cerr << "exec:" << tmp << endl;
    cerr << "bestUsed:" << bestNode.usedColor << "\tbestRecolor:" << bestNode.recolor << endl;
    cerr << "bestScore:" << bestNode.usedColor*bestNode.recolor << endl;
    vector<int> res(R);
    auto node = bestNode;
    while (node.prev != -1) {
      res[node.region] = node.color;
      node = history[node.prev];
    }
    return res;
  }
  vector<int> recolor(int _H, vector<int> _regions, vector<int> _oldColors) {
    startTime = getTime();
    H = _H;
    W = _regions.size() / H;
    R = 0;
    C = 0;
    for (int i=0; i < H*W; i++) {
      R = max(R, _regions[i]+1);
      C = max(C, _oldColors[i]+1);
    }
    area.assign(R, 0);
    for (int i=0; i < H*W; i++) {
      area[_regions[i]]++;
    }
    graph.assign(R, vector<int>());
    cost.assign(R, vector<int>());
    for (int i=0; i < R; i++) {
      cost[i] = vector<int>(R, area[i]);
    }
    for (int i=0; i < H*W; i++) {
      int r = _regions[i];
      int y = i/W;
      int x = i%W;
      for (int j=0; j < 4; j++) {
        int ny = y + dy[j];
        int nx = x + dx[j];
        if (!isin(ny, nx)) continue;
        int nr = _regions[ny*W+nx];
        if (r == nr) continue;
        graph[r].push_back(nr);
        graph[nr].push_back(r);
      }
      for (int j=0; j < C; j++) {
        if (_oldColors[i] == j) cost[r][j]--;
      }
    }
    minRegionColorCost.assign(R, vector<int>((1 << C), 1<<30));
    for (int i=0; i < R; i++) {
      for (int b=0; b < 1 << C; b++) {
        minRegionColorCost[i][b] = cost[i][C];
        for (int c=0; c < C; c++) {
          if ((b >> c) & 1) continue;
          minRegionColorCost[i][b] = min(minRegionColorCost[i][b], cost[i][c]);
        }
      }
    }
    regions.clear();
    for (int i=0; i < R; i++) {
      auto &g = graph[i];
      sort(g.begin(), g.end());
      g.erase(unique(g.begin(), g.end()), g.end());
      regions.push_back(Region(i, graph[i]));
    }
    degree.resize(R);
    for (int i=0; i < R; i++) {
      degree[i] = graph[i].size();
    }
    cerr << "H:" << H << "\tW:" << W << "\tR:" << R << endl;
    return beamSearch();
  }
};
#endif  // MAP_RECOLORING_HPP_
