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
const double timeLimit = 9.5;
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
  bool isin(int y, int x) {
    return 0 <= y && y < H && 0 <= x && x < W;
  }

 public:
  vector<int> recolor(int _H, vector<int> _regions, vector<int> _oldColors) {
    double startTime = getTime();
    H = _H;
    W = _regions.size() / H;
    R = 0;
    C = 0;
    for (int i=0; i < H*W; i++) {
      R = max(R, _regions[i]+1);
      C = max(C, _oldColors[i]+1);
    }
    vector<int> area(R, 0);
    for (int i=0; i < H*W; i++) {
      area[_regions[i]]++;
    }
    vector<vector<int> > graph(R, vector<int>());
    vector<vector<int> > cost(R);
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
        if (r == _regions[ny*W+nx]) continue;
        graph[r].push_back(_regions[ny*W+nx]);
        graph[_regions[ny*W+nx]].push_back(r);
      }
      for (int j=0; j < C; j++) {
        if (_oldColors[i] == j) cost[r][j]--;
      }
    }

    vector<Region> regions;
    for (int i=0; i < R; i++) {
      auto &g = graph[i];
      sort(g.begin(), g.end());
      g.erase(unique(g.begin(), g.end()), g.end());
      regions.push_back(Region(i, graph[i]));
    }

    cerr << "R:" << R << endl;
    while (getTime()-startTime < timeLimit) {
      vector<double> regionWeight(R+1, 0);
      for (int i=0; i < R; i++) {
        regionWeight[i+1] = regionWeight[i] + regions[i].link.size();
      }
      double targetRegionWeight = rng.uniform() * regionWeight[R];
      int targetRegionIndex = lower_bound(
        regionWeight.begin(), regionWeight.end(), targetRegionWeight) - regionWeight.begin() - 1;
      auto &region = regions[targetRegionIndex];
      // cerr << targetRegionWeight << " " << regionWeight[R] << endl;
      // cerr << "targetRegionIndex:" << targetRegionIndex << endl;

      vector<double> colorWeight;
      colorWeight.reserve(R+1);
      colorWeight.push_back(0);
      double targetColorWeight = 0;
      for (int i=0; i < R; i++) {
        colorWeight.push_back(colorWeight[i]);
        if (region.color == i) continue;
        bool sameColor = false;
        for (auto adjRegion : region.link) {
          if (regions[adjRegion].color == i) sameColor = true;
        }
        if (sameColor) continue;
        colorWeight[i+1] += 1.0/cost[targetRegionIndex][i];
        targetColorWeight = colorWeight[i+1];
        if (i >= C) break;
      }
      targetColorWeight *= rng.uniform();
      int targetColor = lower_bound(
        colorWeight.begin(), colorWeight.end(), targetColorWeight) - colorWeight.begin() - 1;
      // cerr << "targetColor:" << targetColor << endl;

      double diff = cost[targetRegionIndex][targetColor] - cost[targetRegionIndex][region.color];
      if (diff < 0) {
        region.color = targetColor;
      }
    }

    vector<int> res;
    for (int i=0; i < R; i++) {
      res.push_back(regions[i].color);
    }
    return res;
  }
};
#endif  // MAP_RECOLORING_HPP_
