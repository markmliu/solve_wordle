#include "utils.h"

#include <iostream>
#include <numeric>
#include <vector>
#include <fstream>


void save_progress(const std::vector<std::pair<int, double>>& diffs) {
  std::ofstream out("hard_mode_diff.checkpoint");
  for (const auto& [count,diff] : diffs) {
    out << count << "," << diff << std::endl;
  }
  out.close();

}

int main() {

  std::vector<std::string> solution_list = load_sol_words();
  std::vector<std::string> guess_list = load_guess_words();

  // all words are still remaining;
  std::vector<int> all_solution_idxs(solution_list.size());
  std::iota(std::begin(all_solution_idxs), std::end(all_solution_idxs), 0);

  std::vector<int> all_guess_idxs(guess_list.size());
  std::iota(std::begin(all_guess_idxs), std::end(all_guess_idxs), 0);

  auto [guess, _] = get_best_word(guess_list, all_guess_idxs, solution_list, all_solution_idxs, /*use_cache=*/true);

  std::vector<std::vector<int>> sol_partitions = partition_space_for_word(guess, solution_list, all_solution_idxs);
  std::vector<std::vector<int>> guess_partitions = partition_space_for_word(guess, guess_list, all_guess_idxs);

  // for each partition, see what word we guess if we:
  //   - have access to all the guess words or
  //   - only have access to the remaining guess words.

  // save, for each non-empty partition, how many words were in the partition and the diff in entropy there.
  std::vector<std::pair<int, double>> diffs;

  for (int i = 0; i < sol_partitions.size(); ++i) {
    std::cout << "checking partition " << i << " of " << (sol_partitions.size() -1 ) << std::endl;
    if (i % 20 == 0) {
      save_progress(diffs);
    }
    if (sol_partitions.at(i).empty()) {
      // skip empty partition or singular partition.
      continue;
    }

    if (sol_partitions.at(i).size() == 1) {
      // no diff in entropy if only one word remaining.
      diffs.push_back(std::make_pair(1,0.0));
      continue;
    }

    auto [constrained_guess, constrained_ent] = get_best_word(guess_list, guess_partitions.at(i), solution_list, sol_partitions.at(i), /*use_cache=*/false);
    auto [unconstrained_guess, unconstrained_ent] = get_best_word(guess_list, all_guess_idxs, solution_list, sol_partitions.at(i), /*use_cache=*/false);
    assert(unconstrained_ent >= constrained_ent);
    diffs.push_back(std::make_pair(sol_partitions.at(i).size(), unconstrained_ent - constrained_ent));
    if (unconstrained_ent > constrained_ent + 0.001) {
      std::cout << "solution space has remaining words:" << sol_partitions.at(i).size() << " for entropy: " << log2(sol_partitions.at(i).size()) << std::endl;
      std::cout << "best constrained guess is " << constrained_guess << " with entropy: " << constrained_ent << std::endl;
      std::cout << "best overall guess is " << unconstrained_guess << " with entropy: " << unconstrained_ent << std::endl;
    }
  }
  save_progress(diffs);

}
