/**
\file

Simple debug logging to stderr.

*/
#include <stdio.h>

#include "common.hpp"

#define debugf(fmt, ...) (fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__))
