#ifndef KOMENDY_H
#define KOMENDY_H

#include "copy.h"
#include "include.h"
#include "sighandler.h"
#include "struct.h"

void add_command(char** argv, int argc, backup kopie[]);
void end_command(char** argv, int argc, backup kopie[]);
void list_command(backup kopie[]);
void restore_command(char**argv,int argc);
void exit_command(backup kopie[]);
#endif
