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

void find_worst_case(const std::vector<std::string>& word_list) {
  // all words are still remaining;
  std::vector<int> remaining_words(word_list.size());
  std::iota(std::begin(remaining_words), std::end(remaining_words), 0);
  auto [guess, _] = get_best_word(word_list, remaining_words, /*use_cache=*/true);
  std::cout << "first guess: " << guess << std::endl;
  std::vector<std::vector<int>> partitions = partition_answer_space_for_word(guess, word_list, remaining_words);
  // std::vector<std::vector<int>> partitions = partition_answer_space_for_word("tares", word_list, remaining_words);
  // we need to break up each partition until they are size 0 or 1.
  int worst_case = 0;
  std::string worst_case_str = "";

  int cur_depth = 1;
  // num partitions before
  std::cout << "num partitions before " << partitions.size() << std::endl;
  partitions.erase(std::remove_if(partitions.begin(), partitions.end(),  [](std::vector<int>& part) { return part.empty();}), partitions.end());
  std::cout << "num partitions after " << partitions.size() << std::endl;
  // num partitions after

  struct Node {
    Node(const std::vector<int>& remaining_words, int depth) : remaining_words(remaining_words), depth(depth) {}
    std::vector<int> remaining_words;
    int depth;
  };
  // generate tree of partitions, use BFS to make sure we traverse in level-wise order. deepest partition will be worst case.
  std::queue<Node> q;
  for (const auto& part : partitions) {
    q.push(Node(part, 1));
  }

  int explored_nodes_at_depth = 0;
  while (!q.empty()) {
    auto& front = q.front();
    explored_nodes_at_depth++;
    std::cout << "exploring node " << explored_nodes_at_depth << " with depth " << front.depth << " and " << front.remaining_words.size() << " words remaining" <<  std::endl;
    if (front.depth > cur_depth) {
      // since we are traversing level-wise, will only be increasing.
      cur_depth = front.depth;
      explored_nodes_at_depth = 0;
    }
    // if only one word left, update the "worst case" and stop exploring
    if (front.remaining_words.size() == 1) {
      if (front.depth + 1 > worst_case) {
	worst_case = front.depth + 1;
	worst_case_str = front.remaining_words.at(0);
      }
      // std::cout << "only 1 word remaining, no need to explore any deeper" << std::endl;
      q.pop();
      continue;
    }
    // figure out what the min entropy guess is from here.
    auto [guess, _] = get_best_word(word_list, front.remaining_words, /*use_cache=*/false);
    std::vector<std::vector<int>> partitions = partition_answer_space_for_word(guess, word_list, front.remaining_words);
    partitions.erase(std::remove_if(partitions.begin(), partitions.end(),  [](std::vector<int>& part) { return part.empty();}), partitions.end());
    // also remove any partitions that are size 1
    auto old_size = partitions.size();
    auto it = std::partition(partitions.begin(), partitions.end(),  [](std::vector<int>& part) { return part.size() != 1;});
    if (it != partitions.end()) {
      std::cout << "removing some 1-size partitions early" << std::endl;
      if (front.depth + 2 > worst_case) {
	worst_case = front.depth + 2;
	worst_case_str = word_list.at(it->at(0));
      }
      partitions.erase(it, partitions.end());
    }

    auto new_size = partitions.size();
    if (old_size != new_size) {
      std::cout << "went from " << old_size << " partitions to " << new_size << " partitions after removing size 1" << std::endl;
    }
    for  (const auto& part : partitions) {
      q.push(Node(part, front.depth + 1));
    }
    q.pop();
    // do stuff here
  }

  std::cout << "max depth: " << worst_case << std::endl;
  std::cout << "worst word: " << worst_case_str << std::endl;

}

int main() {
  // std::map<std::string, double> entrop_dict = load_checkpoint();
  std::vector<std::string> valid_words = load_words();

  find_worst_case(valid_words);

  // assume we guess tares first.
  // for (const auto& word : valid_words) {
  //   auto constraints = resulting_constraints("tares", word);
  // }
}
