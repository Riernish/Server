//
// Created by iliya on 20.03.2021.
//
#include "Error.h"

void m_print_error(const char info[], const char file[], size_t line) {
    perror(info);
    printf("\n File: %s\n Line %lu\n ", file, line );

}