#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>

int sys_write(int fd, const void* buf, size_t count, ssize_t* bytes_written);
int sys_seek(int fd, off_t offset, int whence, off_t* new_offset);
int sys_read(int fd, void* buf, size_t count, ssize_t* bytes_read);
int sys_open(const char* path, int flags, mode_t mode, int* fd);
int sys_openat(int dirfd, const char* path, int flags, mode_t mode, int* fd);
int sys_get_cwd(char* buffer, size_t size);
int sys_close(int fd);
int sys_stat(int fd, const char* path, int flags, struct stat* statbuf);
int sys_read_entries(int fd, void* buffer, size_t max_size, size_t* bytes_read);
int sys_chdir(const char* path);
int sys_mkdir(const char* path, mode_t mode);
