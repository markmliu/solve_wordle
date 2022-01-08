#pragma once

#include <vector>
#include <string>
#include <map>

#include "cppitertools/product.hpp"

// load 5-letter words from sowpods.txt
std::vector<std::string> load_words();

// load short corpus of 12 5-letter words.
std::vector<std::string> load_words_test();

std::map<std::string, double> load_checkpoint();

void save_checkpoint(const std::map<std::string, double>& entrop_dict);

bool has_letter_at_pos(const std::string& word, const char letter, int pos);

bool has_letter_not_at_pos(const std::string& word, const char letter, int pos);

bool does_not_have_letter(const std::string& word, const char letter, int pos);

double calc_entropy_for_word(std::string query, const std::vector<std::string>& all_words, const std::vector<int>& constrained_word_idxs);

std::pair<std::string, double> get_best_word(const std::vector<std::string>& words, const std::vector<int>& constrained_word_idxs, bool use_cache);

// like calc_entropy_for_word but actually returns the partitions of words
std::vector<std::vector<int>> partition_answer_space_for_word(std::string query, const std::vector<std::string>& all_words, const std::vector<int>& remaining_words);
