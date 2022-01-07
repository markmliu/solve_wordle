#include "matplotlibcpp.h"
#include <fstream>
#include <iostream>

namespace plt = matplotlibcpp;

std::map<std::string, double> load_checkpoint() {
  std::ifstream in("wordle.checkpoint");
  if (!in.good()) {
    return {};
  }
  std::string str;
  std::map<std::string, double> entrop_dict;
  while (std::getline(in, str)) {
    int comma_pos = str.find(',');
    entrop_dict[str.substr(0,5)] = std::stod(str.substr(6));
  }
  in.close();
  return entrop_dict;
}


int main() {
  // load the saved entropy values
  std::map<std::string, double> entrop_dict = load_checkpoint();
  // dump to a vector
  std::vector<double> entrops;
  for (const auto& kv : entrop_dict) {
    entrops.push_back(kv.second);
    if (kv.second > 6.0) {
      std::cout << kv.first << " " << kv.second << std::endl;
    }
  }

  plt::hist(entrops);
  // plt::show();
  plt::save("hist.png");
  return 0;
}
