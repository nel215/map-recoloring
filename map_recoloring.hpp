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
const double timeLimit = 9.0;
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
  vector<int> sa() {
    vector<int> usedColor(R, 0);
    double bestUsedColor = 0;
    double currentUsedColor = 0;
    double bestRecolor = 0;
    double currentRecolor = 0;
    for (int i=0; i < R; i++) {
      usedColor[i]++;
      bestUsedColor += 1;
      currentUsedColor += 1;
      bestRecolor += cost[i][i];
      currentRecolor += cost[i][i];
    }

    vector<int> res(R);
    int iteration = 0;
    while (getTime()-startTime < timeLimit) {
      iteration += 1;
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
      double targetUsedColor = currentUsedColor;
      if (usedColor[region.color] == 1) {
        targetUsedColor -= 1;
      }
      if (usedColor[targetColor] == 0) {
        targetUsedColor += 1;
      }
      double targetRecolor = currentRecolor;
      targetRecolor -= cost[targetRegionIndex][region.color];
      targetRecolor += cost[targetRegionIndex][targetColor];

      double prev = bestUsedColor/currentUsedColor * bestRecolor/currentRecolor;
      double next = bestUsedColor/targetUsedColor * bestRecolor/targetRecolor;
      // cerr << prev << " " << next << endl;
      if (next > prev) {
        usedColor[region.color] -= 1;
        usedColor[targetColor] += 1;
        currentUsedColor = targetUsedColor;
        currentRecolor = targetRecolor;
        region.color = targetColor;
        bestRecolor = targetRecolor;
        bestUsedColor = targetUsedColor;
        for (int i=0; i < R; i++) {
          res[i] = regions[i].color;
        }
      } else if (next > rng.uniform()) {
        usedColor[region.color] -= 1;
        usedColor[targetColor] += 1;
        currentUsedColor = targetUsedColor;
        currentRecolor = targetRecolor;
        region.color = targetColor;
      }
    }

    // debug
    // for (int y=0; y < H; y++) {
    //   for (int x=0; x < W; x++) {
    //     int i = y*W+x;
    //     int r = _regions[i];
    //     cerr << '0' + r;
    //   }
    //   cerr << endl;
    // }
    // cerr << endl;
    // for (int y=0; y < H; y++) {
    //   for (int x=0; x < W; x++) {
    //     int i = y*W+x;
    //     int r = _regions[i];
    //     cerr << (regions[r].color >= 10 ? '*' : '0' + regions[r].color);
    //   }
    //   cerr << endl;
    // }

    cerr << "iteration:" << iteration << endl;
    cerr << "bestUsedColor:" << bestUsedColor << endl;
    cerr << "bestRecolor:" << bestRecolor<< endl;
    return res;
  }
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
  vector<int> beamSearch() {
    const int maxColor = 10;
    vector<vector<uint32_t> > hashSeed(R, vector<uint32_t>(maxColor+1));
    for (int i=0; i < R; i++) {
      for (int j=0; j <= maxColor; j++) hashSeed[i][j] = rng.rand();
    }
    vector<priority_queue<Node> > queues(R+1, priority_queue<Node>());
    vector<set<uint32_t> > pushed(R+1, set<uint32_t>());
    vector<Node> history;
    queues[0].push(Node(-1, -1, -1, 0, 0));
    while (getTime()-startTime < timeLimit) {
      for (int q=0; q < R; q++) {
        auto &que = queues[q];
        if (que.empty()) continue;

        // reconstruct
        auto node = que.top();
        uint32_t hash = 0;
        vector<int> usedRegion(R, -1);
        set<int> usedColors;
        while (node.prev != -1) {
          hash ^= hashSeed[node.region][min(node.color, maxColor)];
          usedColors.insert(node.color);
          usedRegion[node.region] = node.color;
          node = history[node.prev];
        }
        node = que.top();
        int prev = history.size();
        history.push_back(node);
        que.pop();

        // sort region
        priority_queue<pair<double, int> > regionQue;
        for (int i=0; i < R; i++) {
          if (usedRegion[i] >= 0) continue;
          // upper is worse for deletion
          double score = -degree[i];
          if (regionQue.size() >= 20) {
            if (regionQue.top().first > score) {
              regionQue.pop();
              regionQue.push(make_pair(score, i));
            }
          } else {
            regionQue.push(make_pair(score, i));
          }
        }

        // select color
        while (!regionQue.empty()) {
          auto idx = regionQue.top().second;
          auto &region = regions[idx];
          regionQue.pop();

          for (int i=0; i < R; i++) {
            uint32_t nextHash = hash ^ hashSeed[idx][min(i, maxColor)];
            if (pushed[q+1].count(nextHash) > 0) continue;

            bool sameColor = false;
            for (auto adjRegion : region.link) {
              if (usedRegion[adjRegion] == i) sameColor = true;
            }
            if (sameColor) continue;

            int recolor = node.recolor + cost[idx][i];
            int usedColor = node.usedColor;
            if (usedColors.count(i) == 0) usedColor++;
            auto next = Node(idx, i, prev, usedColor, recolor);
            auto &nextQue = queues[q+1];
            nextQue.push(next);
            pushed[q+1].insert(nextHash);
            if (i >= C) break;
          }
        }
      }
    }

    cerr << "beam:" << queues[R].size() << endl;
    cerr << "pushed:" << pushed[R].size() << endl;
    vector<int> res(R);
    auto node = queues[R].top();
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
