#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <numeric>

#include "cppitertools/product.hpp"
#include "utils.h"

std::vector<std::function<bool(const std::string&)>> parse_constraints_string(const std::string& constraints_string) {
  if (constraints_string.empty()) {
    return {};
  }
  std::vector<std::function<bool(const std::string&)>> constraints = {};

  std::vector<std::function<bool(const std::string&, const char, int)>> raw_constraints;

  raw_constraints.push_back(has_letter_at_pos);
  raw_constraints.push_back(has_letter_not_at_pos);
  raw_constraints.push_back(does_not_have_letter);

  for (int i = 0; i < constraints_string.size(); i+= 2) {
    char letter = constraints_string.at(i);
    int idx = (int)(constraints_string.at(i+1))-(int)('0') - 1;
    constraints.push_back(std::bind(raw_constraints.at(idx), std::placeholders::_1, letter, i/2));
  }
  return constraints;
}

int main() {
  // get list of words
  std::vector<std::string> words = load_words();
  std::cout << "number of five letter words: " << words.size() << std::endl;

  std::vector<int> constrained_word_idxs(words.size());
  std::iota(std::begin(constrained_word_idxs), std::end(constrained_word_idxs), 0);

  auto [guess,ent] = get_best_word(words, constrained_word_idxs,/*use_cache=*/true);
  std::cout << guess << " has highest entropy of " << ent << std::endl;

  std::string constraints_string;
  std::vector<std::function<bool(const std::string&)>> constraints;
  while (true) {
    std::cout << "Please input constraint string: (ex. t1e2a2r3s3 would mean the word contains a 't' in the correct position, 'e' and 'a' in wrong positions and does not contain 'r' or 's')\n" << std::endl;
    std::cin >> constraints_string;
    std::vector<std::function<bool(const std::string&)>> new_constraints = parse_constraints_string(constraints_string);
    constraints.insert(constraints.end(), new_constraints.begin(), new_constraints.end());
    // recompute constrained_word_idxs
    std::vector<int> temp;
    for (const int idx : constrained_word_idxs) {
      bool failed_constraint = false;
      const auto& word = words.at(idx);
      for (const auto& constraint : constraints) {
	if (!constraint(word)) {
	  failed_constraint = true;
	  break;
	}
      }
      if (!failed_constraint) {
	temp.push_back(idx);
      }
    }
    constrained_word_idxs = temp;
    if (constrained_word_idxs.size() == 1) {
      std::cout << "found only one choice: " << words.at(constrained_word_idxs.at(0)) << std::endl;
      return 0;
    } else if (constrained_word_idxs.size() == 0) {
      std::cout << "no words found matching all constraints. either a bug or vocab isn't big enough" << std::endl;
      return 0;
    }
    auto [next_guess, ent] = get_best_word(words, constrained_word_idxs, /*use_cache=*/false);
    std::cout << "let's guess: " << next_guess << " which has entropy: " << ent << std::endl;
  }
}
