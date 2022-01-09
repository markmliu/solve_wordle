#pragma once

#include <vector>
#include <string>
#include <map>

#include "cppitertools/product.hpp"

// load 5-letter words from sowpods.txt
std::vector<std::string> load_guess_words();

std::vector<std::string> load_sol_words();

// load short corpus of 12 5-letter words.
std::vector<std::string> load_words_test();

std::map<std::string, double> load_checkpoint();

void save_checkpoint(const std::map<std::string, double>& entrop_dict);

bool has_letter_at_pos(const std::string& word, const char letter, int pos);

bool has_letter_not_at_pos(const std::string& word, const char letter, int pos);

bool does_not_have_letter(const std::string& word, const char letter, int pos);

// given the query word and the list of possible solutions, return the entropy.
double calc_entropy_for_word(std::string query, const std::vector<std::string>& all_solutions, const std::vector<int>& constrained_solution_idxs);

std::pair<std::string, double> get_best_word(const std::vector<std::string>& guess_words, const std::vector<int>& constrained_guess_idxs, const std::vector<std::string>& solution_words, const std::vector<int>& constrained_solution_idx, bool use_cache);

// like calc_entropy_for_word but actually returns the partitions of words
std::vector<std::vector<int>> partition_answer_space_for_word(std::string query, const std::vector<std::string>& all_words, const std::vector<int>& remaining_words);
