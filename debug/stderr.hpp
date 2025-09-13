#include <stdio.h>

#include "common.hpp"

#define debugf(fmt, ...) (fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__))
