# Wordle Solver

Finds all possible words given letters that are known to be in the solution
(positions can be specified or unspecified)
and letters that are known to not be in the solution.

## Installation

```
# Clone the repo
$ git clone https://github.com/jstlwy/cpp_wordle_solver.git

# Change the working directory to wordle_solver
$ cd wordle_solver

# Build
$ make
```

## Usage

```
./wordle_solver -exclude m,o,a,c -include u -known 1s,5e
```

## Other recommended word files

- [FreeBSD's /share/dict/words](https://svnweb.freebsd.org/csrg/share/dict/words?revision=61569&view=markup)
- [Infochimps](https://github.com/dwyl/english-words/blob/master/words.txt)
- [Infochimps (no numbers or punctuation)](https://github.com/dwyl/english-words/blob/master/words_alpha.txt)
