#!/bin/bash
mkdir build

cd build

cmake ..

make

./breadscript ../example.bread
