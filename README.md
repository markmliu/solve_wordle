# solve_wordle
solver for [wordle](https://www.powerlanguage.co.uk/wordle/)

## dependencies

* matplotlib
* cppitertools

## usage:

python version: `python solve_wordle.py`

cpp:

# solve the wordle

compile with `g++ solve_wordle.cpp utils.cpp --std=c++14` and run `./a.out`

# plot the entropies

`g++ plot_entropies.cpp --std=c++14 -I/usr/include/python2.7 -lpython2.7`