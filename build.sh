#! /bin/sh

g++ lambda.cpp -o ./build/lambda \
  -std=c++1z \
  -I`pwd`/lib/bdwgc/include `pwd`/lib/bdwgc/lib/libgc.so \
  -Wall \
&& ./build/lambda
