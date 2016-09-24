#! /bin/sh

g++ test.cpp -o ./build/test \
  -std=c++1z \
  -I`pwd`/lib/bdwgc/include `pwd`/lib/bdwgc/lib/libgc.so \
  -Wall \
&& ./build/test
