#include "utils.h"

#include <fstream>
#include <iostream>
#include <numeric>

// entropy of discrete random variable.
double entropy(const std::vector<double>& prob_vec) {
  double ent;
  for (const double prob : prob_vec) {
    if (prob == 0) {
      continue;
    }
    ent += (prob * log2(prob));
  }
  return -1.0 * ent;
}

std::vector<std::string> load_guess_words() {
  std::ifstream in("sowpods.txt");
  std::string str;
  std::vector<std::string> words;
  while (std::getline(in, str)) {
    if (str.size() == 5) {
      words.push_back(str);
    }
  }
  return words;
}

std::vector<std::string> load_sol_words() {
  std::ifstream in("solutions.txt");
  std::string str;
  std::vector<std::string> words;
  while (std::getline(in, str)) {
    if (str.size() == 5) {
      words.push_back(str);
    }
  }
  return words;
}



std::vector<std::string> load_words_test_small() {
  return {
    "apple",
    "baker",
    "candy",
    "dolly",
    "event",
    "facet",
    "gates",
    "hairy",
    "igloo",
    "allow"
  };
}

// std::vector<std::string> load_words_test_medium() {
// }

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

void save_checkpoint(const std::map<std::string, double>& entrop_dict) {
  std::ofstream out("wordle.checkpoint");
  for (const auto& [k,v] : entrop_dict) {
    out << k << "," << v << std::endl;
  }
  out.close();
}

bool has_letter_at_pos(const std::string& word, const char letter, int pos) {
  return word.at(pos) == letter;
}

bool has_letter_not_at_pos(const std::string& word, const char letter, int pos) {
  return word.at(pos) != letter && word.find(letter) != std::string::npos;
}

bool does_not_have_letter(const std::string& word, const char letter, int pos) {
  return word.find(letter) == std::string::npos;
}

double calc_entropy_for_word(std::string query, const std::vector<std::string>& all_words, const std::vector<int>& constrained_word_idxs) {
  std::vector<std::function<bool(const std::string&, const char, int)>> fs;

  // has_letter_at_pos
  fs.push_back(has_letter_at_pos);
  fs.push_back(has_letter_not_at_pos);
  fs.push_back(does_not_have_letter);

  std::vector<double> counts;
  for (const auto& [f0,f1,f2,f3,f4] : iter::product(fs,fs,fs,fs,fs)) {
    int count = 0;
    for (const auto idx : constrained_word_idxs) {
      const auto& word = all_words.at(idx);
      if (f0(word, query.at(0), 0) &&
	  f1(word, query.at(1), 1) &&
	  f2(word, query.at(2), 2) &&
	  f3(word, query.at(3), 3) &&
	  f4(word, query.at(4), 4)) {
	count++;
      }
    }
    counts.push_back(count);
  }
  std::for_each(counts.begin(), counts.end(), [&](double &c) { c /= constrained_word_idxs.size();});
  return entropy(counts);
}

std::pair<std::string, double> get_best_word(const std::vector<std::string>& guess_words, const std::vector<int>& constrained_guess_idxs, const std::vector<std::string>& sol_words, const std::vector<int>& constrained_sol_idxs, bool use_cache) {
  std::map<std::string, double> entrop_dict = {};
  if (use_cache) {
    entrop_dict = load_checkpoint();
  }

  int count = 0;
  for (const auto idx : constrained_guess_idxs) {
    const auto& query = guess_words.at(idx);
    if (entrop_dict.find(query) != entrop_dict.end()) {
      count++;
      continue;
    }
    entrop_dict[query] = calc_entropy_for_word(query, sol_words, constrained_sol_idxs);
    count++;
    // std::cout << query << " has entropy: " << entrop_dict[query] << std::endl;
    if (use_cache && count % 20 == 0) {
      std::cout << "saving checkpoint at " << count << " words" << std::endl;
      save_checkpoint(entrop_dict);
    }
  }

  if (use_cache) {
    save_checkpoint(entrop_dict);
  }

  // find max
  return *(std::max_element(entrop_dict.begin(), entrop_dict.end(),
			    [](const std::pair<std::string, double>& p1, const std::pair<std::string, double>& p2) {
			      return p1.second < p2.second; }));
}

// like calc_entropy_for_word but actually returns the partitions of words
std::vector<std::vector<int>> partition_answer_space_for_word(std::string query, const std::vector<std::string>& all_words, const std::vector<int>& remaining_words) {
  std::vector<std::function<bool(const std::string&, const char, int)>> fs;

  // has_letter_at_pos
  fs.push_back(has_letter_at_pos);
  fs.push_back(has_letter_not_at_pos);
  fs.push_back(does_not_have_letter);

  std::vector<std::vector<int>> partitions;
  for (const auto& [f0,f1,f2,f3,f4] : iter::product(fs,fs,fs,fs,fs)) {
    std::vector<int> part;
    for (int idx : remaining_words) {
      const auto& word = all_words.at(idx);
      if (f0(word, query.at(0), 0) &&
	  f1(word, query.at(1), 1) &&
	  f2(word, query.at(2), 2) &&
	  f3(word, query.at(3), 3) &&
	  f4(word, query.at(4), 4)) {
	part.push_back(idx);
      }
    }
    partitions.push_back(part);
  }
  // look at the partition
  // for (const auto& part : partitions) {
  //   std::cout << part.size() << ",";
  // }
  // std::cout << std::endl;
  return partitions;
}
