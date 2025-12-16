#include "watcher.h"
// source: wyklad zdarzenia watch_tree.c
void add_to_map(struct WatchMap *map, int wd, const char *path)
{
    if (map->watch_count >= MAX_WATCHES)
    {
        fprintf(stderr, "Exceeded max watches!\n");
        return;
    }
    map->watch_map[map->watch_count].wd = wd;
    map->watch_map[map->watch_count].path = strdup(path);
    map->watch_count++;
}

struct Watch *find_watch(struct WatchMap *map, int wd)
{
    for (int i = 0; i < map->watch_count; i++)
    {
        if (map->watch_map[i].wd == wd)
        {
            return &map->watch_map[i];
        }
    }
    return NULL;
}

void remove_from_map(struct WatchMap *map, int wd)
{
    for (int i = 0; i < map->watch_count; i++)
    {
        if (map->watch_map[i].wd == wd)
        {
            free(map->watch_map[i].path);
            map->watch_map[i] = map->watch_map[map->watch_count - 1];
            map->watch_count--;
            return;
        }
    }
}

void add_watch_recursive(int notify_fd, struct WatchMap *map, const char *base_path)
{
    uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE;

    int wd = inotify_add_watch(notify_fd, base_path, mask);
    if (wd < 0)
    {
        return;
    }
    add_to_map(map, wd, base_path);

    DIR *dir = opendir(base_path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        struct stat st;
        if (lstat(full_path, &st) == 0 && S_ISDIR(st.st_mode))
        {
            add_watch_recursive(notify_fd, map, full_path);
        }
    }
    closedir(dir);
}
