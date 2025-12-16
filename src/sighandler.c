#include "sighandler.h"
volatile sig_atomic_t last_sig = 0;

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;

    int res = sigaction(sigNo, &act, NULL);

    if (res == -1 && errno == EINVAL)
        return;

    if (res == -1)
        ERR("sigaction");
}
void sig_handler(int sig) { last_sig = sig; }
void sig_ignore()
{
    for (int i = 1; i < NSIG; i++)
    {
        if (i == SIGCHLD || i == SIGSTOP || i == SIGKILL) 
            continue;
        sethandler(SIG_IGN, i);
    }
    sethandler(sig_handler, SIGINT);
    sethandler(sig_handler, SIGTERM);
}
