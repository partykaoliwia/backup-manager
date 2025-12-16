#ifndef STRUCT_H
#define STRUCT_H

#include "include.h"

#define MAX_KOPIE 100 //maksymalna liczba procesow

typedef struct
{
    pid_t pid;           // ID procesu (0 oznacza, że slot jest pusty)
    char src[PATH_MAX];  // Ścieżka źródłowa
    char dst[PATH_MAX];  // Ścieżka docelowa
} backup;

void initstruct(backup kopie[]);
void insert(pid_t pid, backup kopie[], char* src, char* dst);

#endif
