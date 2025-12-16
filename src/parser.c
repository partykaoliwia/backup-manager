#include "parser.h"

int parse(char* line, char** argv)
{
    static char argumenty[MAX_ARGS][PATH_MAX];
    int argc = 0;
    int i = 0;

    while (line[i] != '\0' && line[i] != '\n' && argc < MAX_ARGS)
    {
        // Pomijam spacje i tabulator
        while (line[i] == ' ' || line[i] == '\t')
        {
            i++;
        }
        if (line[i] == '\0' || line[i] == '\n')
            break;
        int j = 0;
        int in_quote = 0;
        char* current_arg = argumenty[argc];

        // Obsluga cudzyslowia
        while (line[i] != '\0' && line[i] != '\n')
        {
            if (line[i] == '"')
            {
                in_quote = !in_quote;
            }
            else if (line[i] == ' ' && !in_quote)
            {
                break;
            }
            else
            {
                if (j < PATH_MAX - 1)
                {
                    current_arg[j] = line[i];
                    j++;
                }
            }
            i++;
        }
        current_arg[j] = '\0';
        argv[argc] = current_arg;
        argc++;
    }

    return argc;
}
