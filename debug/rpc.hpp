#include <stdio.h>

#define debugf(fmt, ...) (fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__))
