#include "map_recoloring.hpp"


int main() {
  int H, nRegions, nColors;
  cin >> H;
  cin >> nRegions;
  vector<int> regions(nRegions);
  for (int i = 0; i < nRegions; ++i) {
    cin >> regions[i];
  }
  cin >> nColors;
  vector<int> oldColors(nColors);
  for (int i = 0; i < nColors; ++i) {
    cin >> oldColors[i];
  }

  MapRecoloring mr;
  vector<int> ret = mr.recolor(H, regions, oldColors);
  cout << ret.size() << endl;
  for (int i = 0; i < static_cast<int>(ret.size()); ++i) {
    cout << ret[i] << endl;
  }
  cout.flush();
}

