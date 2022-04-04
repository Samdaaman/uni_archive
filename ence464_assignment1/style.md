# Style Guide


## Introduction
This style guide is short to make it readable. The style guidance is based on [Recommended C Style and Coding Standards](https://www.doc.ic.ac.uk/lab/cplus/cstyle.html), which will from here be refered to as the general style guide. Use it for topics not covered in this guide, and if a rule in this document contradicts a rule in the general style guide, use the rule in this document.


## [Program and header file structure](https://www.doc.ic.ac.uk/lab/cplus/cstyle.html#N100D8)
No change from the general style guide.


## [Naming conventions](https://www.doc.ic.ac.uk/lab/cplus/cstyle.html#N103FD)
- `#define` constants are in `ALL_CAPS`
- Enum constants, enum tags, struct names, and typedefs for structs are `Capitalized`
- File, function, and variable names are in `snake_case`
- Where applicable, prefer American English


## Braces and indentation
- Open braces for functions are on their own line
- Flow control statement braces are on the same line as the statement, separated by a space
- Close braces are on their own line
- Switch statements have cases **and** statements indented


## [Whitespace](https://www.doc.ic.ac.uk/lab/cplus/cstyle.html#N10247)
The example in the general style guide is inconsistent with the text.

- Two blank lines between parts of files and between functions
- One blank line can be used within functions
- No space around the boolean expression of if statements
- Spaces around operators


## [Comments](https://www.doc.ic.ac.uk/lab/cplus/cstyle.html#N10126)
- Use doxygen to generate comment skeleton
- All files and functions must be commented, with an exception made for the main() function
- Functions that are declared in a header files and implemented in a program file should be commented in the header file
- Additional comments on the function's implementation should be in the program file


## Example
```C
/**
 * @file my_file.c
 * @author Group 16
 * @brief Description of my_file.c
 */


#include "my_file.h"
#include "my_other_file.h"


#define MY_CONSTANT 1


/**
 * @brief Description of foo
 * 
 * @param bar Description of bar
 */
void foo(int bar)
{
    /* Statements */

    if (bool1) {
        /* Statements */
    } else {
        /* Statements */
    }
}


/**
 * @brief Description of implementation
 */
char baz()
{
    switch (expr) {
        case ABC:
            /* Statements */
            /* FALLTHROUGH */
        case DEF:
            /* Statements */
            break;
    }
}
```




