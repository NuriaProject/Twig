# CMake file for LEMON, the LALR(1) parser generator of SQLite.
cmake_minimum_required(VERSION 2.8.8)

PROJECT(Lemon C CXX)
add_executable(lemon lemon.c)
