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
const double timeLimit = 9.6;
#endif  // LOCAL
inline double getTime() {
    uint32_t lo, hi;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return (((uint64_t)hi << 32) | lo) / ticks_per_sec;
}

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
  vector<Region> regions;
  bool isin(int y, int x) {
    return 0 <= y && y < H && 0 <= x && x < W;
  }

 public:
  struct Node {
    int region;
    int color;
    int prev;
    int usedColor;
    int recolor;
    Node(int _region, int _color, int _prev, int _usedColor, int _recolor) {
      region = _region;
      color = _color;
      prev = _prev;
      usedColor = _usedColor;
      recolor = _recolor;
    }
    bool operator<(const Node &n)const {
      return usedColor*recolor > n.usedColor*n.recolor;
    }
  };
  int getRegionScore(int filled, int degree) {
    return filled*100 + degree;
  }
  vector<int> beamSearch() {
    vector<priority_queue<Node> > queues(R+1, priority_queue<Node>());
    vector<Node> history;
    queues[0].push(Node(-1, -1, -1, 0, 0));
    int tmp = 0;
    auto bestNode = Node(-1, -1, -1, 32, H*W);
    while (getTime()-startTime < timeLimit) {
      for (int q=0; q < R; q++) {
        auto &que = queues[q];
        if (que.empty()) continue;
        tmp++;

        // reconstruct
        auto node = que.top();
        vector<int> usedRegion(R, -1);
        set<int> usedColors;
        while (node.prev != -1) {
          usedColors.insert(node.color);
          usedRegion[node.region] = node.color;
          node = history[node.prev];
        }
        node = que.top();
        int prev = history.size();
        history.push_back(node);
        que.pop();

        // select region
        double bestScore = 0;
        int bestIdx = 0;
        for (int i=0; i < R; i++) {
          if (usedRegion[i] >= 0) continue;
          int filled = 0;
          for (auto r : regions[i].link) {
            if (usedRegion[r] == -1) continue;
            filled |= 1 <<  usedRegion[r];
          }
          filled = __builtin_popcount(filled);
          double score = getRegionScore(filled, degree[i]);
          if (bestScore < score) {
            bestScore = score;
            bestIdx = i;
          }
        }

        // select color
        auto &region = regions[bestIdx];
        for (int i=0; i < 32; i++) {
          bool sameColor = false;
          for (auto adjRegion : region.link) {
            if (usedRegion[adjRegion] == i) sameColor = true;
          }
          if (sameColor) continue;

          int recolor = node.recolor + cost[bestIdx][i];
          int usedColor = node.usedColor;
          if (usedColors.count(i) == 0) usedColor++;
          auto next = Node(bestIdx, i, prev, usedColor, recolor);
          if (next < bestNode) continue;
          auto &nextQue = queues[q+1];
          nextQue.push(next);
          if (i >= C) break;
        }
      }
      // TODO: optimization
      auto &lastQue = queues[R];
      if (!lastQue.empty()) {
        auto node = lastQue.top();
        if (bestNode < node) {
          bestNode = node;
          cerr << "improve" << " " << node.usedColor << endl;
        }
      }
      while (!lastQue.empty()) lastQue.pop();
    }

    cerr << "exec:" << tmp << endl;
    cerr << "bestUsed:" << bestNode.usedColor << "\tbestRecolor:" << bestNode.recolor << endl;
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
