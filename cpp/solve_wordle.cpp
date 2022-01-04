#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <numeric>

#include "cppitertools/product.hpp"

std::vector<std::string> load_words() {
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

bool has_letter_at_pos(const std::string& word, const char letter, int pos) {
  return word.at(pos) == letter;
}

bool has_letter_not_at_pos(const std::string& word, const char letter, int pos) {
  return word.at(pos) != letter && word.find(letter) != std::string::npos;
}

bool does_not_have_letter(const std::string& word, const char letter, int pos) {
  return word.find(letter) == std::string::npos;
}

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


double calc_entropy_for_word(std::string query, const std::vector<std::string>& words) {
  std::vector<std::function<bool(const std::string&, const char, int)>> fs;

  // has_letter_at_pos
  fs.push_back(has_letter_at_pos);
  fs.push_back(has_letter_not_at_pos);
  fs.push_back(does_not_have_letter);

  std::vector<double> counts;
  for (const auto& [f0,f1,f2,f3,f4] : iter::product(fs,fs,fs,fs,fs)) {
    int count = 0;
    for (const auto& word : words) {
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
  std::for_each(counts.begin(), counts.end(), [&](double &c) { c /= words.size();});
  // for (const auto el : counts) {
  //   std::cout.precision(2);
  //   std::cout << el << " ";
  // }
  // std::cout << std::endl;
  // std::cout << "sum of prob vector: " << accumulate(counts.begin(), counts.end(), 0.0) << std::endl;
  return entropy(counts);
}

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

std::pair<std::string, double> get_best_word(const std::vector<std::string>& words, bool use_cache = true) {
  std::map<std::string, double> entrop_dict = {};
  if (use_cache) {
    entrop_dict = load_checkpoint();
  }
  for (int i = 0; i < words.size(); ++i) {
    const auto& query = words.at(i);
    if (entrop_dict.find(query) != entrop_dict.end()) {
      continue;
    }
    entrop_dict[query] = calc_entropy_for_word(query, words);
    std::cout << query << " has entropy: " << entrop_dict[query] << std::endl;
    if (use_cache && i % 20 == 0) {
      std::cout << "saving checkpoint at " << i << " words" << std::endl;
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

int main() {
  // get list of words
  std::vector<std::string> words = load_words();
  std::cout << "number of five letter words: " << words.size() << std::endl;

  auto [guess,ent] = get_best_word(words);
  std::cout << guess << " has highest entropy of " << ent << std::endl;

  std::vector<std::string> constrained_words = words;
  std::string constraints_string;
  std::vector<std::function<bool(const std::string&)>> constraints;
  while (true) {
    std::cout << "Please input constraint string: (ex. t1e2a2r3s3 would mean the word contains a 't' in the correct position, 'e' and 'a' in wrong positions and does not contain 'r' or 's')\n" << std::endl;
    std::cin >> constraints_string;
    std::vector<std::function<bool(const std::string&)>> new_constraints = parse_constraints_string(constraints_string);
    constraints.insert(constraints.end(), new_constraints.begin(), new_constraints.end());
    // recompute words;
    std::vector<std::string> temp;
    for (const auto& word : constrained_words) {
      bool failed_constraint = false;
      for (const auto& constraint : constraints) {
	if (!constraint(word)) {
	  failed_constraint = true;
	  break;
	}
      }
      if (!failed_constraint) {
	temp.push_back(word);
      }
    }
    constrained_words = temp;
    if (constrained_words.size() == 1) {
      std::cout << "found only one choicee: " << constrained_words.at(0) << std::endl;
      return 0;
    } else if (constrained_words.size() == 0) {
      std::cout << "no words found matching all constraints. either a bug or vocab isn't big enough" << std::endl;
      return 0;
    }
    auto [next_guess, ent] = get_best_word(constrained_words, /*use_cache=*/false);
    std::cout << "let's guess: " << next_guess << " which has entropy: " << ent << std::endl;
  }
}
