/**
\file

Simple debug logging to stdout.

*/
#include <stdio.h>

#include "common.hpp"

#define debugf(fmt, ...) (fprintf(stdout, fmt __VA_OPT__(,) __VA_ARGS__))
