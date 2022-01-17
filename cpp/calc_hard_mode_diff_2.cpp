#include "utils.h"

#include <iostream>
#include <numeric>
#include <vector>
#include <fstream>

std::vector<std::pair<int, double>> load_progress() {
  std::ifstream in("hard_mode_diff.checkpoint");
  if (!in.good()) {
    return {};
  }
  std::string str;
  std::vector<std::pair<int, double>> diffs;
  while (std::getline(in, str)) {
    int comma_pos = str.find(',');
    diffs.push_back(std::make_pair(std::stoi(str.substr(0, comma_pos)), std::stod(str.substr(comma_pos+1))));
  }
  in.close();
  return diffs;
}

int main() {
  auto diffs = load_progress();
  // sanity checks.
  std::vector<std::string> solution_list = load_sol_words();

  int total_counts = 0;
  double total_advantage = 0.0;
  int num_partitions_advantage = 0;
  int num_solutions_advantage = 0;
  for (const auto& [count,diff] : diffs) {
    // std::cout << "count: " << count << " diff: " << diff << std::endl;
    total_counts += count;
    total_advantage += (count * diff);
    if (diff > 0.001) {
      num_partitions_advantage++;
      num_solutions_advantage+= count;
    }
  }
  assert(total_counts == solution_list.size());
  total_advantage /= (solution_list.size());
  std::cout << "total advantage: " << total_advantage << std::endl;
  std::cout << "number of solution words that have diff: " << num_solutions_advantage << " out of " << total_counts << std::endl;
}
