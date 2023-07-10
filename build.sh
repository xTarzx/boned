#!/bin/bash


CC="clang++"
CFLAGS="-Wno-narrowing"
# CFLAGS="-Wall -Wextra -Werror=return-type -Wdeprecated"
LIBS="-lraylib -lGL -lm -lpthread -ldl"
OUTFILE="boned"

set -xe

$CC $CFLAGS -o $OUTFILE main.cpp $LIBS
