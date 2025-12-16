#include "struct.h"

void initstruct(backup kopie[])
{
    for (int i = 0; i < MAX_KOPIE; i++)
    {
        kopie[i].pid = 0;
    }
}
void insert(pid_t pid, backup kopie[], char* src, char* dst)
{
    for (int i = 0; i < MAX_KOPIE; i++)
    {
        if (kopie[i].pid == 0)
        {
            kopie[i].pid = pid;
            strcpy(kopie[i].src, src);
            strcpy(kopie[i].dst, dst);
            break;
        }
    }
}
