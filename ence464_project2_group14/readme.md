
ENCE461 Assignment 2 Template
=============================

See assignment instructions [here](doc/instructions.pdf)

Contents
 - `doc/` - assignment instructions, lab notes, report template.
 - `reference/` - correct output for test comparison.
 - `poisson.c` - basic template to work from. Write your solution here.
 - `threads.c` - example on how to use POSIX thread library.
 - `test.sh` - automatic testing script.

Building
--------

Build instructions are inside `poisson.c` and `threads.c`. You can automate this
with makefiles if you want.


Testing
-------

Build your solution.

Run `./test.sh`. *This scripts expects a `poisson` program to be present.*

It will automatically run your solution for three cube sizes and compare the
output against some correct reference files. **Do not edit these reference
files!**