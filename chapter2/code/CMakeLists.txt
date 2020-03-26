cmake_minimum_required(VERSION 2.8.12)
project(storage)

add_definitions("-std=c++11")

add_library(storage SHARED Storage.cpp)