#include "copy.h"
#include "include.h"
#include "komendy.h"
#include "parser.h"
#include "sighandler.h"
#include "struct.h"

void usage()
{
    printf("---------------------- Zarządzanie backupem ----------------------\n");
    printf("Komendy:\n");
    printf("  add <source> <target1> <target2> - uruchamia tworzenie kopii\n");
    printf("  end <source> <target> - przerywa tworzenie kopii\n");
    printf("  list - wypisuje foldery mające kopie zapasowe\n");
    printf("  restore <source> <target> - przywraca z target do source \n");
    printf("  exit - kończy pracę programu\n");
    printf("------------------------------------------------------------------\n");
}
int main()
{
    backup kopie[MAX_KOPIE];
    initstruct(kopie);
    usage();

    size_t len = 0;
    char* argv[MAX_ARGS];
    int argc = 0;
    char* line = NULL;

    sig_ignore();
    sethandler(sig_handler, SIGINT);
    sethandler(sig_handler, SIGTERM);

    while (last_sig != SIGINT && last_sig != SIGTERM)
    {
        int status;
        pid_t ended_pid;
        while((ended_pid=waitpid(-1,&status,WNOHANG))>0)
        {
            printf("Proces [%d] zakończył działanie\n", ended_pid);
            for (int i = 0; i < MAX_KOPIE; i++) 
            {
                if (kopie[i].pid == ended_pid) 
                {
                    kopie[i].pid = 0;
                    break;
                }
            }
        }
        printf("> ");
        if (getline(&line, &len, stdin) > 1)
        {
            argc = parse(line, argv);
            if (argc > 0)
            {
                if (strcmp("exit", argv[0]) == 0)
                {
                    break;
                }
                else if (strcmp("add", argv[0]) == 0)
                {
                    add_command(argv, argc, kopie);
                }
                else if (strcmp("end", argv[0]) == 0)
                {
                    end_command(argv, argc, kopie);
                }
                else if(strcmp("restore",argv[0])==0)
                {
                    restore_command(argv,argc);
                }
                else if (strcmp("list", argv[0]) == 0)
                {
                    list_command(kopie);
                }
                else
                {
                    printf("%s jest nieprawidłową komendą\n", argv[0]);
                }
                
            }
        }
    }
    fprintf(stdout, "\nkoniec pracy programu\n");
    exit_command(kopie);
    free(line);
    exit(EXIT_SUCCESS);
}
