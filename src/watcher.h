#ifndef WATCHER_H
#define WATCHER_H

#include "include.h"

#define MAX_WATCHES 8192
#define EVENT_BUF_LEN (64 * (sizeof(struct inotify_event) + NAME_MAX + 1))

//source: wyklad zdarzenia watch_tree.c
struct Watch
{
    int wd;      // watch descriptor (klucz)
    char *path;  // ścieżka (wartość)
};

struct WatchMap
{
    struct Watch watch_map[MAX_WATCHES];
    int watch_count;
};

void add_to_map(struct WatchMap *map, int wd, const char *path);
void add_watch_recursive(int notify_fd, struct WatchMap *map, const char *base_path);
struct Watch *find_watch(struct WatchMap *map, int wd);
void remove_from_map(struct WatchMap *map, int wd);

#endif
