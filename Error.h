//
// Created by iliya on 20.03.2021.
//

#ifndef CLIENT_ERROR_H
#define CLIENT_ERROR_H

#include <stdio.h>
#include <stdlib.h>



void m_print_error(const char info[], const char file[], size_t line);
#define ERROR(str) m_print_error(str, __FILE__, __LINE__)

#endif //CLIENT_ERROR_H
