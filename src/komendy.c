#include "komendy.h"

void add_command(char** argv, int argc, backup kopie[])
{
    if (argc < 3)
    {
        fprintf(stderr, "ERROR: Podano nieprawidłową liczbę argumentów dla add\n");
        return;
    }

    char src[PATH_MAX];
    // Źródło nie istnieje
    if (realpath(argv[1], src) == NULL)
    {
        perror("Błąd ścieżki źródłowej");
        return;
    }

    for (int i = 2; i < argc; i++)
    {
        char dst[PATH_MAX];
        getabsolute(argv[i], dst);  // Ścieżka bezwzględna
        if (strncmp(dst, src, strlen(src)) == 0)
        {
            if (dst[strlen(src)] == '/' || dst[strlen(src)] == '\0')
            {
                fprintf(stderr, "ERROR: Katalog docelowy nie może być wewnątrz źródłowego!");
                continue;
            }
        }
        // Istnieje już kopia
        if (copyexists(kopie, src, dst) == 1)
        {
            fprintf(stderr, "ERROR: Kopia już istnieje!");
            continue;
        }

        if (checkdir(dst) == -1)
            continue;

        pid_t pid = fork();
        switch (pid)
        {
            case 0:  // dziecko
                childwork(src, dst);
                exit(EXIT_SUCCESS);
            case -1:
                perror("fork");
                break;
            default:  // rodzic
                insert(pid, kopie, src, dst);
                break;
        }
    }
}
void exit_command(backup kopie[])
{
    for (int i = 0; i < MAX_KOPIE; i++)
    {
        if (kopie[i].pid != 0)
        {
            kill(kopie[i].pid, SIGTERM);
            waitpid(kopie[i].pid, NULL, 0);
            kopie[i].pid = 0;
        }
    }
}

void list_command(backup kopie[])
{
    printf("------------------------- Aktywne procesy ------------------------\n");
    int count = 0;
    for (int i = 0; i < MAX_KOPIE; i++)
    {
        if (kopie[i].pid != 0)
        {
            fprintf(stdout, "[%d] | %s -> %s\n", kopie[i].pid, kopie[i].src, kopie[i].dst);
            count++;
        }
    }
    if (count == 0)
        fprintf(stdout, "Brak aktywnych procesów kopiowania :(\n");
    printf("------------------------------------------------------------------\n");
}
void end_command(char** argv, int argc, backup kopie[])
{
    if (argc < 2)
    {
        fprintf(stderr, "ERROR: Za mało argumentów dla end\n");
        return;
    }
    char src_abs[PATH_MAX];
    getabsolute(argv[1], src_abs);

    for (int j = 2; j < argc; j++)
    {
        char dst_abs[PATH_MAX];
        getabsolute(argv[j], dst_abs);
        int found = 0;
        for (int i = 0; i < MAX_KOPIE; i++)
        {
            if (kopie[i].pid != 0 && strcmp(kopie[i].src, src_abs) == 0 && strcmp(kopie[i].dst, dst_abs) == 0)
            {
                fprintf(stdout, "Koniec procesu [%d] (%s -> %s)\n", kopie[i].pid, src_abs, dst_abs);
                kill(kopie[i].pid, SIGTERM);
                waitpid(kopie[i].pid, NULL, 0);
                kopie[i].pid = 0;
                found = 1;
                break;
            }
        }
        if (found == 0)
        {
            fprintf(stdout, "Nie znaleziono aktywnej kopii\n");
        }
    }
}
void restore_command(char**argv,int argc)
{
    if(argc<3)
    {
        fprintf(stderr, "ERROR: nieprawidlowa liczba argumentow");
        return;
    }
    char src[PATH_MAX];
    char dst[PATH_MAX];
    getabsolute(argv[1], src);
    getabsolute(argv[2], dst);
    printf("Przywracam z %s do %s\n", dst, src);

    //Czyscimy katalog zrodlowy z plikow, ktorych nie ma w backupie(dst)
    clean(src, dst);
    copy_dir(dst, src, dst, src);
}
