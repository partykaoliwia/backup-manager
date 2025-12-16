#ifndef SIGHANDLER_H
#define SIGHANDLER_H

#include "include.h"

extern volatile sig_atomic_t last_sig;
void sethandler(void (*f)(int), int sigNo);
void sig_handler(int sig);
void sig_ignore();
#endif
