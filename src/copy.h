#ifndef COPY_H
#define COPY_H

#include "include.h"
#include "sighandler.h"
#include "struct.h"
#include "watcher.h"

ssize_t bulk_read(int fd, char *buf, size_t count);
ssize_t bulk_write(int fd, char *buf, size_t count);
void copy_file(char *src, char *dst);
void copy_dir(char *src_path, char *dst_path, char *src_initial, char *dst_initial);
void copy_symlink(char *src_path, char *dst_path, char *src_initial, char *dst_initial);
int checkdir(char *path);
int copyexists(backup kopie[], char *src, char *dst);

void childwork(char *src, char *dst);
void getabsolute(char *path, char *buff);
void clean(char *src, char *dst); // dla restorea
void remove_recursive(const char *path);



#endif
