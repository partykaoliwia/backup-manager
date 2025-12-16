#include "copy.h"
#define BUFF_SIZE 4096

// source:L1
ssize_t bulk_read(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

// source:L1
ssize_t bulk_write(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

// Kopiuję plik z src do dst
void copy_file(char *src, char *dst)
{
    const int fd_src = open(src, O_RDONLY);
    if (fd_src == -1)
    {
        perror("open src");
        return;
    }
    struct stat st;
    if (fstat(fd_src, &st) == -1)
    {
        perror("fstat src");
        if (close(fd_src) == -1)
            perror("close src");
        return;
    }

    const int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (fd_dst == -1)
    {
        perror("open dst");
        if (close(fd_src) == -1)
            perror("close src");
        return;
    }
    if (fchmod(fd_dst, st.st_mode) == -1)
    {
        perror("fchmod");
    }

    char buf[BUFF_SIZE];

    // source:L1
    for (;;)
    {
        const ssize_t read_size = bulk_read(fd_src, buf, BUFF_SIZE);
        if (read_size == -1)
        {
            fprintf(stderr, "bulk_read");
            break;
        }

        if (read_size == 0)
            break;

        if (bulk_write(fd_dst, buf, read_size) == -1)
        {
            fprintf(stderr, "bulk_write");
            break;
        }
    }

    if (close(fd_src) == -1)
        perror("close src");

    if (close(fd_dst) == -1)
        perror("close dst");
}

void copy_symlink(char *src_path, char *dst_path, char *src_initial, char *dst_initial)
{
    char link[PATH_MAX];
    // Odczytujemy, gdzie wskazuje symlink
    ssize_t len = readlink(src_path, link, sizeof(link) - 1);
    if (len == -1)
    {
        perror("readlink");
        return;
    }
    link[len] = '\0';

    char new_link[PATH_MAX];

    // Ściezka bezwzględna, wnętrze kopiowanego katalogu
    if (link[0] == '/' && strncmp(link, src_initial, strlen(src_initial)) == 0)
    {
        char *suffix = link + strlen(src_initial);
        snprintf(new_link, PATH_MAX, "%s%s", dst_initial, suffix);
    }
    else
    {
        // Ścieżka
        strncpy(new_link, link, PATH_MAX);
    }

    // Tworzymy symlink w miejscu docelowym
    if (symlink(new_link, dst_path) == -1 && errno != EEXIST)
    {
        perror("symlink");
    }
}

void copy_dir(char *src_path, char *dst_path, char *src_initial, char *dst_initial)
{
    // Otwieram katalog źródłowy
    DIR *src = opendir(src_path);
    if (src == NULL)
    {
        perror("opendir");
        return;
    }

    struct dirent *file_info;

    // Czytam wszystkie pliki w directory
    while ((file_info = readdir(src)) != NULL)
    {
        // Ignoruję "." i ".."
        if (strcmp(file_info->d_name, ".") == 0 || strcmp(file_info->d_name, "..") == 0)
        {
            continue;
        }

        // Tworzę pełne ścieżki
        char new_src[PATH_MAX];
        char new_dst[PATH_MAX];
        snprintf(new_src, PATH_MAX, "%s/%s", src_path, file_info->d_name);
        snprintf(new_dst, PATH_MAX, "%s/%s", dst_path, file_info->d_name);

        // Pobieramy file status
        struct stat st;
        if (lstat(new_src, &st) == -1)
        {
            perror("lstat");
            continue;
        }

        // Folder
        if (S_ISDIR(st.st_mode))
        {
            if (mkdir(new_dst, st.st_mode) != 0 && errno != EEXIST)
            {
                perror("mkdir");
                continue;
            }
            chmod(new_dst, st.st_mode);
            copy_dir(new_src, new_dst, src_initial, dst_initial);
        }
        // Zwykły plik
        else if (S_ISREG(st.st_mode))
        {
            copy_file(new_src, new_dst);
        }
        // Symlink
        else if (S_ISLNK(st.st_mode))
        {
            copy_symlink(new_src, new_dst, src_initial, dst_initial);
        }
    }
    if (closedir(src) == -1)
        perror("closedir");
}
int checkdir(char *path)
{
    errno = 0;
    DIR *dir = opendir(path);
    // Nie istnieje katalog
    if (dir == NULL && errno == ENOENT)
    {
        if (mkdir(path, 0777) != 0)
        {
            perror("mkdir");
            return -1;
        }
        return 0;
    }
    // Brak uprawnień
    if (dir == NULL && errno == EACCES)
    {
        perror("opendir");
        return -1;
    }
    // Ścieżka nie jest katalogiem
    if (dir == NULL && errno == ENOTDIR)
    {
        perror("opendir");
        return -1;
    }

    // Sprawdzamy, czy docelowy jest pusty
    struct dirent *file_info;
    while ((file_info = readdir(dir)) != NULL)
    {
        if (strcmp(file_info->d_name, ".") == 0 || strcmp(file_info->d_name, "..") == 0)
        {
            continue;
        }
        fprintf(stderr, "ERROR: Katalog docelowy nie jest pusty!!\n");
        closedir(dir);
        return -1;
    }
    closedir(dir);
    return 0;
}

int copyexists(backup kopie[], char *src, char *dst)
{
    for (int i = 0; i < MAX_KOPIE; i++)
    {
        if (kopie[i].pid != 0)
        {
            if (strcmp(kopie[i].src, src) == 0 && strcmp(kopie[i].dst, dst) == 0)
            {
                return 1;  // Kopia istnieje
            }
        }
    }
    return 0;  // Brak kopii
}

// większość z wykładu o eventach, watch_tree.c
void childwork(char *src, char *dst)
{
    sethandler(SIG_IGN, SIGINT);
    copy_dir(src, dst, src, dst);
    //Inicjalizacja inotify
    int notify_fd = inotify_init();
    if (notify_fd < 0)
    {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }
    struct WatchMap map = {0};
    //Watchery rekursywnie
    add_watch_recursive(notify_fd, &map, src);
    char buffer[EVENT_BUF_LEN];
    while (last_sig != SIGTERM)
    {
        ssize_t len = read(notify_fd, buffer, EVENT_BUF_LEN);
        if (len < 0)
        {
            if (errno!=EINTR)
                perror("read");
            break;
        }
        struct stat st_check;
        //Sytuacja, gdy folder docelowy zostanie usunięty
        if (lstat(dst, &st_check) == -1 && errno == ENOENT)
        {
            struct stat st_s;
            if(lstat(src, &st_s) == 0) 
            {
                printf("Naprawa folderu docelowego: %s\n", dst);
                mkdir(dst, st_s.st_mode);
                copy_dir(src, dst, src, dst);
            }
        }
        ssize_t i = 0;
        while (i < len)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            struct Watch *watch = find_watch(&map, event->wd);
            if(watch)
            {
                //Folder zrodlowy usuniety
                if (event->mask & IN_DELETE_SELF || event->mask & IN_MOVE_SELF || event->mask & IN_IGNORED)
                {
                    if (strcmp(watch->path, src) == 0) 
                    {
                        printf("[%d] Katalog źródłowy został usunięty.\n", getpid());
                        close(notify_fd);
                        exit(EXIT_SUCCESS); 
                    }
                    remove_from_map(&map, event->wd); //tylko podkatalog
                }
                //Zmiany
                else if (event->len > 0)
                {
                    char full_src_path[PATH_MAX];
                    char full_dst_path[PATH_MAX];

                    // Budujemy ścieżkę
                    snprintf(full_src_path, PATH_MAX, "%s/%s", watch->path, event->name);
                    size_t len_src = strlen(src);
                    char *suffix = full_src_path + len_src;
                    snprintf(full_dst_path, PATH_MAX, "%s%s", dst, suffix);

                    // Usunięty lub wyciągnięty
                    if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM)
                    {
                        printf("[%d] Usuwanie: %s\n", getpid(), full_dst_path);
                        if (event->mask & IN_ISDIR)
                        {
                            remove_recursive(full_dst_path);
                        }
                        else
                        {
                            unlink(full_dst_path);
                        }
                    }

                    // Stworzony lub wrzucony (katalog)
                    else if ((event->mask & IN_CREATE || event->mask & IN_MOVED_TO) && (event->mask & IN_ISDIR))
                    {
                        printf("[%d] Nowy katalog: %s\n", getpid(), full_dst_path);
                        mkdir(full_dst_path, 0777);
                        add_watch_recursive(notify_fd, &map, full_src_path);
                        copy_dir(full_src_path, full_dst_path, src, dst);
                    }

                    // Modyfikacje, MODIFY by psulo
                    else if (event->mask & IN_CLOSE_WRITE || event->mask & IN_MOVED_TO || event->mask & IN_CREATE)
                    {
                        printf("[%d] Aktualizacja pliku: %s\n", getpid(), full_dst_path);
                        struct stat st;
                        if (lstat(full_src_path, &st) == 0) 
                        {
                            if (S_ISLNK(st.st_mode))
                                copy_symlink(full_src_path, full_dst_path, src, dst);
                            else
                                copy_file(full_src_path, full_dst_path);
                        }
                    }
                }
            }
            i += sizeof(struct inotify_event) + event->len;
        }
    }
    //sprzatanko
    close(notify_fd);
    for (int j = 0; j < map.watch_count; j++)
    {
        free(map.watch_map[j].path);
    }
    exit(EXIT_SUCCESS);
}

void getabsolute(char *path, char *buff)
{
    // //Ścieżki typu ~/cośtam
    // if(path[0]=='~')
    // {
    //     char*home=getenv("HOME");
    //     if(home!=NULL)
    //     {
    //         if(snprintf(buff,PATH_MAX,"%s%s",home,path+1)>=PATH_MAX)
    //             fprintf(stderr, "ERROR: getabsolute() ścieżka z ~ zbyt długa");
    //         return;
    //     }
    // }

    // Istniejąca ścieżka
    if (realpath(path, buff) != NULL)
    {
        return;
    }

    // Ścieżka jeszcze nie istnieje, więc ją tworzymy
    if (path[0] == '/')
    {
        // Ścieżka bezwzględna
        strncpy(buff, path, PATH_MAX);
    }
    else
    {
        // Ścieżka względna
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            perror("getcwd");
            return;
        }
        if (snprintf(buff, PATH_MAX, "%s/%s", cwd, path) >= PATH_MAX)
        {
            fprintf(stderr, "ERROR: getabsolute() ścieżka wynikowa za długa");
            return;
        }
    }
}
void remove_recursive(const char *path)
{
    DIR *d = opendir(path);
    if (d==NULL) return;

    struct dirent *p;
    while ((p = readdir(d)) != NULL)
    {
        if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
            continue;

        char subpath[PATH_MAX];
        snprintf(subpath, sizeof(subpath), "%s/%s", path, p->d_name);

        struct stat st;
        if (lstat(subpath, &st) == -1) continue;

        if (S_ISDIR(st.st_mode))
        {
            remove_recursive(subpath);
        }
        else
        {
            unlink(subpath);
        }
    }
    closedir(d);
    rmdir(path); // Na końcu usuwamy sam katalog
}
void clean(char *src, char *dst)
{
    DIR *d_src = opendir(src);
    if (!d_src) return;

    struct dirent *file;
    while ((file = readdir(d_src)) != NULL)
    {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
            continue;

        char path_src[PATH_MAX];
        char path_dst[PATH_MAX];

        snprintf(path_src, PATH_MAX, "%s/%s", src, file->d_name);
        snprintf(path_dst, PATH_MAX, "%s/%s", dst, file->d_name);

        // Sprawdzamy, czy plik ze źródła istnieje w backupie
        struct stat st_dst;
        // Nie ma w backupie to usuwamy ze źródła
        if (lstat(path_dst, &st_dst) == -1 && errno == ENOENT)
        {
            struct stat st_src;
            lstat(path_src, &st_src);
            if (S_ISDIR(st_src.st_mode))
                remove_recursive(path_src);
            else
                unlink(path_src);
        }
        // Plik jest w backupie. Jeśli to katalog, to wchodzimy dalej
        else
        {
            struct stat st_src;
            lstat(path_src, &st_src);
            if (S_ISDIR(st_src.st_mode))
            {
                clean(path_src, path_dst);
            }
        }
    }
    closedir(d_src);
}