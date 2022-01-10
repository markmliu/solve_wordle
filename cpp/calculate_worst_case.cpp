#include "utils.h"

#include <map>
#include <string>
#include <iostream>
#include <numeric>
#include <queue>
#include <vector>

#include "assert.h"


std::vector<std::function<bool(const std::string&)>> resulting_constraints(const std::string& guess, const std::string& true_word) {
  assert(guess.size() == 5);
  assert(true_word.size() == 5);

  std::vector<std::function<bool(const std::string&)>> constraints;

  for (int i = 0; i < 5; ++i) {
    char guess_letter = guess.at(i);
    if (true_word.at(i) == guess_letter) {
      constraints.push_back([i, guess_letter](const std::string& w) { return w.at(i) == guess_letter; });
    } else if (true_word.find(guess_letter) != std::string::npos){
      constraints.push_back([i, guess_letter](const std::string& w) { return w.at(i) != guess_letter && w.find(guess_letter) != std::string::npos;});
    } else {
      constraints.push_back([guess_letter](const std::string& w) { return w.find(guess_letter) == std::string::npos;});
    }
  }
  return constraints;
}

void find_worst_case(const std::vector<std::string>& solution_list,
		     const std::vector<std::string>& guess_list) {
  // all words are still remaining;
  std::vector<int> constrained_solution_idxs(solution_list.size());
  std::iota(std::begin(constrained_solution_idxs), std::end(constrained_solution_idxs), 0);

  std::vector<int> constrained_guess_idxs(guess_list.size());
  std::iota(std::begin(constrained_guess_idxs), std::end(constrained_guess_idxs), 0);

  auto [guess, _] = get_best_word(guess_list, constrained_guess_idxs, solution_list, constrained_solution_idxs, /*use_cache=*/true);
  // TODO: need to constrain the guess list for each node also.
  std::cout << "first guess: " << guess << std::endl;
  std::vector<std::vector<int>> sol_partitions = partition_space_for_word(guess, solution_list, constrained_solution_idxs);
  std::vector<std::vector<int>> guess_partitions = partition_space_for_word(guess, guess_list, constrained_guess_idxs);

  // zip up the partitions.
  assert(sol_partitions.size() == guess_partitions.size());
  std::vector<std::pair<std::vector<int>, std::vector<int>>> partitions;
  for (int i = 0; i < sol_partitions.size(); ++i) {
    partitions.push_back(std::make_pair(sol_partitions.at(i), guess_partitions.at(i)));
  }
  // partitions.at(i).first is solutions, partitions.at(i).second is guesses.

  // we need to break up each partition until the remaining sols are size 0 or 1.
  int worst_case = 0;
  std::string worst_case_str = "";

  int cur_depth = 1;
  // num partitions before
  std::cout << "num partitions before " << partitions.size() << std::endl;
  partitions.erase(std::remove_if(partitions.begin(), partitions.end(),  [](std::pair<std::vector<int>, std::vector<int>>& part) { return part.first.empty();}), partitions.end());
  std::cout << "num partitions after " << partitions.size() << std::endl;
  // num partitions after

  struct Node {
    Node(const std::vector<int>& constrained_solution_idxs, const std::vector<int>& constrained_guess_idxs, int depth) : constrained_solution_idxs(constrained_solution_idxs), constrained_guess_idxs(constrained_guess_idxs), depth(depth) {}
    std::vector<int> constrained_solution_idxs;
    std::vector<int> constrained_guess_idxs;
    int depth;
  };
  // generate tree of partitions, use BFS to make sure we traverse in level-wise order. deepest partition will be worst case.
  std::queue<Node> q;
  for (const auto& part : partitions) {
    q.push(Node(part.first, part.second, 1));
  }

  int explored_nodes_at_depth = 0;
  while (!q.empty()) {
    auto& front = q.front();
    explored_nodes_at_depth++;
    std::cout << "exploring node " << explored_nodes_at_depth << " with depth " << front.depth << " and " << front.constrained_solution_idxs.size() << " solution words remaining and " << front.constrained_guess_idxs.size() << " guess words remaining." <<  std::endl;
    if (front.depth > cur_depth) {
      // since we are traversing level-wise, will only be increasing.
      cur_depth = front.depth;
      explored_nodes_at_depth = 0;
    }
    // if only one word left, update the "worst case" and stop exploring
    if (front.constrained_solution_idxs.size() == 1) {
      if (front.depth + 1 > worst_case) {
	worst_case = front.depth + 1;
	worst_case_str = front.constrained_solution_idxs.at(0);
      }
      // std::cout << "only 1 word remaining, no need to explore any deeper" << std::endl;
      q.pop();
      continue;
    }
    // figure out what the min entropy guess is from here.
    auto [guess, _] = get_best_word(guess_list, constrained_guess_idxs, solution_list, front.constrained_solution_idxs, /*use_cache=*/false);
    std::vector<std::vector<int>> sol_partitions = partition_space_for_word(guess, solution_list, front.constrained_solution_idxs);
    std::vector<std::vector<int>> guess_partitions = partition_space_for_word(guess, guess_list, front.constrained_guess_idxs);

    // zip up the partitions.
    assert(sol_partitions.size() == guess_partitions.size());
    std::vector<std::pair<std::vector<int>, std::vector<int>>> partitions;
    for (int i = 0; i < sol_partitions.size(); ++i) {
      partitions.push_back(std::make_pair(sol_partitions.at(i), guess_partitions.at(i)));
    }

    partitions.erase(std::remove_if(partitions.begin(), partitions.end(),  [](std::pair<std::vector<int>, std::vector<int>>& part) { return part.first.empty();}), partitions.end());
    // also remove any partitions that are size 1
    auto old_size = partitions.size();
    auto it = std::partition(partitions.begin(), partitions.end(),  [](std::pair<std::vector<int>, std::vector<int>>& part) { return part.first.size() != 1;});
    if (it != partitions.end()) {
      std::cout << "removing some 1-size partitions early" << std::endl;
      if (front.depth + 2 > worst_case) {
	worst_case = front.depth + 2;
	worst_case_str = solution_list.at(it->first.at(0));
      }
      partitions.erase(it, partitions.end());
    }

    auto new_size = partitions.size();
    if (old_size != new_size) {
      std::cout << "went from " << old_size << " partitions to " << new_size << " partitions after removing size 1" << std::endl;
    }
    for  (const auto& part : partitions) {
      q.push(Node(part.first, part.second, front.depth + 1));
    }
    q.pop();
    // do stuff here
  }

  std::cout << "max depth: " << worst_case << std::endl;
  std::cout << "worst word: " << worst_case_str << std::endl;

}

int main() {
  // std::map<std::string, double> entrop_dict = load_checkpoint();
  std::vector<std::string> sol_words = load_sol_words();
  std::vector<std::string> guess_words = load_guess_words();

  find_worst_case(sol_words, guess_words);

  // assume we guess tares first.
  // for (const auto& word : valid_words) {
  //   auto constraints = resulting_constraints("tares", word);
  // }
}
