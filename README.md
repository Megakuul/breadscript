# Breadscript - Interpreted language, created just for fun.

This language aims to parse a file into a Abstract Syntax Tree and then traverse the AST and execute the whole programm.

### Installation

-> Not done as the interpreter is not in a state where it can be used.

### Syntax

-> Not done as the interpreter is not in a state where it can be used.

### Code-infrastructure

#### Components

The interpreter is split up in multiple components:

 - Lexer
 - Parser
 - Driver
 - Evaluator

Currently everything is in one file, if it grows in complexity I will split it up...

#### Error handling

As the compiler should always directly throw an error and exit if an illegal action is done in the code, I made the controversial decision to use **goto**. I only use a single **goto** statement, that goes to the end of the main function where used variables are deallocated and the program is exited.

I might be taught a lesson and change it to use structured control flow, but for me, this single **goto** statement is just so beautiful in this specific case.
