import string
import pickle
import sys

from itertools import product
from scipy.stats import entropy
from os.path import exists
from functools import partial

def load_words():
    # with open('words_alpha.txt') as word_file:
    with open('sowpods.txt') as word_file:
        valid_words = list(word_file.read().split())

    return valid_words

def has_letter_at_pos(word, letter, pos):
    return word[pos] == letter

def has_letter_not_at_pos(word, letter, pos):
    if has_letter_at_pos(word, letter, pos):
        return False
    return word.find(letter) != -1

def does_not_have_letter(word, letter, pos):
    return word.find(letter) == -1

def calc_entropy_for_word(query, all_words):
    # what are all the ways things could turn out?
    # - 243 combinations
    # - 12.5k words
    # - 5 checks
    # - thats ~ 15 million checks per word...
    counts = []
    for comb in product([has_letter_at_pos, has_letter_not_at_pos, does_not_have_letter], repeat=5):
        count = 0
        for word in all_words:
            if all([comb[i](word, query[i], i) for i in range(5)]):
                # print("found one!" + word)
                count += 1
        counts.append(count)
    probs = [float(count) / len(all_words) for count in counts]
    # print(probs[0:10])
    return entropy(probs, base=2)

def parse_constraints_string(constraints_string):
    if len(constraints_string) == 0:
        return []
    constraints = []
    constraint_order = [has_letter_at_pos, has_letter_not_at_pos, does_not_have_letter]
    for i in range(0, len(constraints_string), 2):
        letter = constraints_string[i]
        idx = int(constraints_string[i+1])-1
        constraints.append(partial(constraint_order[idx],letter=letter, pos=i/2))
    return constraints

def get_best_word(word_list, use_cache):
    entrop_dict = {}
    if use_cache and exists('entrop.pickle'):
        with open('entrop.pickle', 'rb') as handle:
            entrop_dict = pickle.load(handle)
            print("loading entropy from pickle with length: ", len(entrop_dict))


    count = len(entrop_dict) # use this for checkpointing
    for query in word_list:
        if entrop_dict.has_key(query):
            # print("skipping query that's already computed: ", query)
            continue
        entrop_dict[query] = calc_entropy_for_word(query, word_list)
        if use_cache and count % 20 == 0:
            print(query, " has entropy: ", entrop_dict[query])
            print("checkpointing entropy at count: ", count)
            with open('entrop.pickle', 'wb') as handle:
                pickle.dump(entrop_dict, handle, protocol=pickle.HIGHEST_PROTOCOL)
        count += 1

    best_word = max(entrop_dict, key=entrop_dict.get)
    entrop = entrop_dict[best_word]
    return (best_word, entrop)

if __name__ == '__main__':
    english_words = load_words()

    # count number of 5-letter words
    five_letter_words = [word for word in english_words if len(word) == 5]
    print(len(five_letter_words))

    first_guess, entrop = get_best_word(five_letter_words, use_cache=True)
    print("let's guess: ", first_guess, " which has entropy: ", entrop)

    constrained_words = five_letter_words
    constraints = []
    while True:
        constraints_string = raw_input("Please input constraint string: (ex. t1e2a2r3s3 would mean the word contains a 't' in the correct position, 'e' and 'a' in wrong positions and does not contain 'r' or 's')\n")
        # todo: validation checking
        new_constraints = parse_constraints_string(constraints_string)
        constraints.extend(new_constraints)
        constrained_words = [word for word in constrained_words if all([constraint(word) for constraint in constraints])]

        if len(constrained_words) == 0:
            print("no words found matching all constraints :(")
            sys.exit()
        elif len(constrained_words) == 1:
            print("found only one choice: ", constrained_words[0])
            sys.exit()

        next_guess, entrop = get_best_word(constrained_words, use_cache=False)
        print("let's guess: ", next_guess, "which has entropy: ", entrop, " while max entropy for this list of words is ", entropy([1.0/len(constrained_words) for i in range(len(constrained_words))], base=2))
