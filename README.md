# TaskMiner - A Source to Source Compiler for Parallelizing Irregular C/C++ Programs with Code Annotation

[Project Webpage](http://cuda.dcc.ufmg.br/taskminer/)

[Code Repository](https://github.com/gleisonsdm/TaskMiner/)

## Introduction

### Motivation

Nowadays, the high performance of computers and smartphones is one of the most important aspects to users. On the other hand, the demands of information are growing day by day and, consequently, the programs need to process larger sets of data. The problem is that people would like to process more data using less time than before, and the industry cannot improve more the hardware as before. One of the most promisses solutions to solve this kind of problem is use more CPU's cores, trying to spend less time processing different peaces of data at the same time.

Unfortunately, a large portion of the apps and programs that people use eachday was not struturized to use all the processment power available. People have powerfull equipments, but they cannot obtain a large speedup, due the major part of the programers are not friendly with the required specificities to process data in parallel. 

Recently, programers have an important ally, some metalanguages as OpenACC and OpenMP. These metalanguages can simplifying the necessary process to write high performance programs, they provide some level of abstraction, and helps to structurize the correct execution order of functions in the program. Even tought this kind of abstraction, programers still needing to define the parallel regions, and their data dependences, directive-based models don't find data dependences, they just organize them.

These kind of programming models have his shortcomings, as is necessary to use specific libraries to write programs that sectorize the data and process them concurrently. And, even if the programmer is adepted to explore the computer especificities and its aspects, identify the data dependencies in the program is a hard task, subject to erros. 

### Implementation

We have developed TaskMiner as a tool to automate the performance of these tasks. Through the implementation of a static analysis that derives memory access bounds from source code, it infers the size of memory regions in C and C++ programs. With these bounds, it is capable of inserting data copy directives in the original source code. These directives provide a compatible compiler with information on which data must be used in each portion of the program. Given the source code of a program as input, our tool can then provide the user with a modified version containing directives with a mapping of regions that are attractive to paralelize, proper their memory bounds specified, all without any further intervention from the user, effectively freeing developers from the burdensome task of manual code modification. 

We implemented TaskMiner as a collection of compiler modules, or passes, for the LLVM compiler infrastructure, whose code is available in this repository.

## Functionality

### Compiler Directive Standards
