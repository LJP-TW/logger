#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/stat.h>

#define BUFSIZE 0x200

static int inited;
static int ofd;

static int (*real_chmod)(const char *pathname, mode_t mode);
static int (*real_chown)(const char *pathname, uid_t owner, gid_t group);
static int (*real_close)(int fd);
static int (*real_creat)(const char *pathname, mode_t mode);
static int (*real_fclose)(FILE *stream);
static FILE *(*real_fopen)(const char *pathname, const char *mode);
static size_t (*real_fread)(void *ptr, size_t size, size_t nmemb,
                    FILE *stream);
static size_t (*real_fwrite)(const void *ptr, size_t size, size_t nmemb,
                    FILE *stream);
static int (*real_open)(const char *pathname, int flags, mode_t mode);
static ssize_t (*real_read)(int fd, void *buf, size_t count);
static int (*real_remove)(const char *pathname);
static int (*real_rename)(const char *oldpath, const char *newpath);
static FILE *(*real_tmpfile)(void);
static ssize_t (*real_write)(int fd, const void *buf, size_t count);

static void logger_init(void)
{
    if (inited) {
        return;
    }

    inited = 1;

    ofd = atoi(getenv("OUTPUT_FILE_FD"));

    real_chmod = dlsym(RTLD_NEXT, "chmod");
    real_chown = dlsym(RTLD_NEXT, "chown");
    real_close = dlsym(RTLD_NEXT, "close");
    real_creat = dlsym(RTLD_NEXT, "creat");
    real_fclose = dlsym(RTLD_NEXT, "fclose");
    real_fopen = dlsym(RTLD_NEXT, "fopen");
    real_fread = dlsym(RTLD_NEXT, "fread");
    real_fwrite = dlsym(RTLD_NEXT, "fwrite");
    real_open = dlsym(RTLD_NEXT, "open");
    real_read = dlsym(RTLD_NEXT, "read");
    real_remove = dlsym(RTLD_NEXT, "remove");
    real_rename = dlsym(RTLD_NEXT, "rename");
    real_tmpfile = dlsym(RTLD_NEXT, "tmpfile");
    real_write = dlsym(RTLD_NEXT, "write");
}

int chmod(const char *pathname, mode_t mode)
{
    logger_init();

    int ret;
    const char *p;
    char realpath_buf[BUFSIZE];

    p = realpath(pathname, realpath_buf);
    if (!p) {
        p = pathname;
    }

    ret = real_chmod(pathname, mode);

    dprintf(ofd, "[logger] chmod(\"%s\", %03o) = %d\n", p, mode, ret);

    return ret;
}

int chown(const char *pathname, uid_t owner, gid_t group)
{
    logger_init();

    int ret;
    const char *p;
    char realpath_buf[BUFSIZE];

    p = realpath(pathname, realpath_buf);
    if (!p) {
        p = pathname;
    }

    ret = real_chown(pathname, owner, group);

    dprintf(ofd, "[logger] chown(\"%s\", %d, %d) = %d\n", 
            p, owner, group, ret);

    return ret;
}

int close(int fd)
{
    logger_init();

    int ret, len;
    char fdpath[BUFSIZE];
    char path[BUFSIZE];

    snprintf(fdpath, BUFSIZE, "/proc/self/fd/%d", fd);
    if ((len = readlink(fdpath, path, BUFSIZE - 1)) == -1) {
        path[0] = 0;
    }
    path[len] = 0;

    ret = real_close(fd);

    dprintf(ofd, "[logger] close(\"%s\") = %d\n", path, ret);

    return ret;
}

int creat(const char *pathname, mode_t mode)
{
    logger_init();

    int ret;
    const char *p;
    char realpath_buf[BUFSIZE];

    p = realpath(pathname, realpath_buf);
    if (!p) {
        p = pathname;
    }

    ret = real_creat(pathname, mode);

    dprintf(ofd, "[logger] creat(\"%s\", %03o) = %d\n", p, mode, ret);

    return ret;
}

int fclose(FILE *stream)
{
    logger_init();

    int ret, fd, len;
    char fdpath[BUFSIZE];
    char path[BUFSIZE];

    fd = fileno(stream);

    snprintf(fdpath, BUFSIZE, "/proc/self/fd/%d", fd);
    if ((len = readlink(fdpath, path, BUFSIZE - 1)) == -1) {
        path[0] = 0;
    }
    path[len] = 0;

    ret = real_fclose(stream);

    dprintf(ofd, "[logger] fclose(\"%s\") = %d\n", path, ret);

    return ret;
}

FILE *fopen(const char *pathname, const char *mode)
{
    logger_init();

    FILE *ret;
    const char *p;
    char realpath_buf[BUFSIZE];

    p = realpath(pathname, realpath_buf);
    if (!p) {
        p = pathname;
    }

    ret = real_fopen(pathname, mode);

    dprintf(ofd, "[logger] fopen(\"%s\", \"%s\") = %p\n", p, mode, ret);

    return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    logger_init();

    size_t ret;
    int len, fd;
    char fdpath[BUFSIZE];
    char path[BUFSIZE];

    fd = fileno(stream);

    snprintf(fdpath, BUFSIZE, "/proc/self/fd/%d", fd);
    if ((len = readlink(fdpath, path, BUFSIZE - 1)) == -1) {
        path[0] = 0;
    }
    path[len] = 0;

    ret = real_fread(ptr, size, nmemb, stream);

    dprintf(ofd, "[logger] fread(\"");

    len = 0;
    while (len < 32) {
        int c = *((char *)ptr + len);
        
        if (!c) {
            break;
        } else if (isprint(c)) {
            dprintf(ofd, "%c", c);
        } else {
            dprintf(ofd, ".");
        }

        len++;
    }
    
    dprintf(ofd, "\", %ld, %ld, \"%s\") = %ld\n", size, nmemb, path, ret);

    return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    logger_init();

    size_t ret;
    int len, fd;
    char fdpath[BUFSIZE];
    char path[BUFSIZE];

    fd = fileno(stream);

    snprintf(fdpath, BUFSIZE, "/proc/self/fd/%d", fd);
    if ((len = readlink(fdpath, path, BUFSIZE - 1)) == -1) {
        path[0] = 0;
    }
    path[len] = 0;

    ret = real_fwrite(ptr, size, nmemb, stream);

    dprintf(ofd, "[logger] fwrite(\"");

    len = 0;
    while (len < 32) {
        int c = *((char *)ptr + len);
        
        if (!c) {
            break;
        } else if (isprint(c)) {
            dprintf(ofd, "%c", c);
        } else {
            dprintf(ofd, ".");
        }

        len++;
    }
    
    dprintf(ofd, "\", %ld, %ld, \"%s\") = %ld\n", size, nmemb, path, ret);

    return ret;
}

int open(const char *pathname, int flags, ...)
{
    logger_init();

    int ret, mode;
    va_list ap;
    const char *p;
    char realpath_buf[BUFSIZE];

    p = realpath(pathname, realpath_buf);
    if (!p) {
        p = pathname;
    }

    va_start(ap, flags);

    if (flags & O_CREAT) {
        mode = va_arg(ap, int);
    } else {
        mode = 0;
    }

    ret = real_open(pathname, flags, mode);

    dprintf(ofd, "[logger] open(\"%s\", %03o, %03o) = %d\n", 
            p, flags, mode, ret);

    va_end(ap);

    return ret;
}

ssize_t read(int fd, void *buf, size_t count)
{
    logger_init();

    ssize_t ret;
    int len;
    char fdpath[BUFSIZE];
    char path[BUFSIZE];

    snprintf(fdpath, BUFSIZE, "/proc/self/fd/%d", fd);
    if ((len = readlink(fdpath, path, BUFSIZE - 1)) == -1) {
        path[0] = 0;
    }
    path[len] = 0;

    ret = real_read(fd, buf, count);

    dprintf(ofd, "[logger] read (\"%s\", \"", path);

    len = 0;
    while (len < 32) {
        int c = *((char *)buf + len);
        
        if (!c) {
            break;
        } else if (isprint(c)) {
            dprintf(ofd, "%c", c);
        } else {
            dprintf(ofd, ".");
        }

        len++;
    }

    dprintf(ofd, "\", %ld) = %ld\n", count, ret);

    return ret; 
}

int remove(const char *pathname)
{
    logger_init();

    int ret;
    const char *p;
    char realpath_buf[BUFSIZE];

    p = realpath(pathname, realpath_buf);
    if (!p) {
        p = pathname;
    }

    ret = real_remove(pathname);

    dprintf(ofd, "[logger] remove(\"%s\") = %d\n", p, ret);

    return ret;
}

int rename(const char *oldpath, const char *newpath)
{
    logger_init();

    int ret;
    const char *p1, *p2;
    char realpath1[BUFSIZE];
    char realpath2[BUFSIZE];

    p1 = realpath(oldpath, realpath1);
    if (!p1) {
        p1 = oldpath;
    }

    p2 = realpath(newpath, realpath2);
    if (!p2) {
        p2 = newpath;
    }

    ret = real_rename(oldpath, newpath);

    dprintf(ofd, "[logger] rename(\"%s\", \"%s\") = %d\n", p1, p2, ret);

    return ret;
}

FILE *tmpfile(void)
{
    logger_init();

    FILE *ret;

    ret = real_tmpfile();

    dprintf(ofd, "[logger] tmpfile() = %p\n", ret);

    return ret;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    logger_init();

    ssize_t ret;
    int len;
    char fdpath[BUFSIZE];
    char path[BUFSIZE];

    snprintf(fdpath, BUFSIZE, "/proc/self/fd/%d", fd);
    if ((len = readlink(fdpath, path, BUFSIZE - 1)) == -1) {
        path[0] = 0;
    }
    path[len] = 0;

    ret = real_write(fd, buf, count);

    dprintf(ofd, "[logger] write(\"%s\", \"", path);

    len = 0;
    while (len < 32) {
        int c = *((char *)buf + len);
        
        if (!c) {
            break;
        } else if (isprint(c)) {
            dprintf(ofd, "%c", c);
        } else {
            dprintf(ofd, ".");
        }

        len++;
    }

    dprintf(ofd, "\", %ld) = %ld\n", count, ret);

    return ret; 
}