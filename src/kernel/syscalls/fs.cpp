#include <kernel/syscalls/fs.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/terminal.hpp>
#include <fcntl.h>
#include <errno.h>

int sys_write(int fd, const void* buf, size_t count, ssize_t* bytes_written) {
    auto task = Scheduler::get_current_task();

    Vfs::FileDescriptor* vfs_fd = task->get_file_handle(fd);
    if (vfs_fd == nullptr) {
        return -EBADF;
    }

    int bytes = Vfs::write(vfs_fd, buf, count);
    if (bytes == -1) {
        return -EBADF;
    }

    *bytes_written = bytes;
    return 0;
}

int sys_seek(int fd, off_t offset, int whence, off_t* new_offset) {
    auto task = Scheduler::get_current_task();

    Vfs::FileDescriptor* vfs_fd = task->get_file_handle(fd);
    if (vfs_fd == nullptr) {
        Terminal::printf("{red}Can't find fd %d\n", fd);
        return -EBADF;
    }

    Vfs::SeekMode mode;
    switch (whence) {
        case SEEK_SET:
            mode = Vfs::SeekMode::Set;
            break;
        case SEEK_CUR:
            mode = Vfs::SeekMode::Current;
            break;
        case SEEK_END:
            mode = Vfs::SeekMode::End;
            break;
        default:
            Terminal::printf("{red}Not supported whence %d\n", whence);
            return -EINVAL;
    }

    Vfs::seek(vfs_fd, offset, mode);

    *new_offset = vfs_fd->rw_position;
    return 0;
}

int sys_read(int fd, void* buf, size_t count, ssize_t* bytes_read) {
    auto task = Scheduler::get_current_task();

    Vfs::FileDescriptor* vfs_fd = task->get_file_handle(fd);
    if (vfs_fd == nullptr) {
        return -EBADF;
    }

    int bytes = Vfs::read(vfs_fd, buf, count);
    if (bytes == -1) {
        return -EBADF;
    }

    *bytes_read = bytes;
    return 0;
}

int sys_open(const char* path, int flags, [[gnu::unused]] mode_t mode, int* fd) {
    Vfs::OpenMode open_mode;
    switch (flags) {
        case O_RDONLY:
            open_mode = Vfs::OpenMode::ReadOnly;
            break;
        case O_WRONLY:
            open_mode = Vfs::OpenMode::WriteOnly;
            break;
        case O_RDWR:
        default:
            open_mode = Vfs::OpenMode::ReadWrite;
            break;
    }

    Vfs::FileDescriptor* vfs_fd = Vfs::open(path, open_mode);
    if (vfs_fd == nullptr) {
        return -EBADF;
    }

    auto task = Scheduler::get_current_task();
    int id = task->add_file_handle(vfs_fd);
    *fd = id;

    return 0;
}

int sys_openat(int dirfd, const char* path, int flags, mode_t mode, int* fd) {
    if (mode != 0) {
        Terminal::printf("{red}(openat) Mode not supported!\n");
        return -ENOSYS;
    } else if (dirfd != AT_FDCWD) {
        Terminal::printf("{red}(dirfd) Dirfd not supported!\n");
        return -ENOSYS;
    }

    auto task = Scheduler::get_current_task();
    char* absolute_path = strdup(task->working_dir);
    strcat(absolute_path, path);

    Vfs::OpenMode open_mode;
    switch (flags) {
        case O_RDONLY:
            open_mode = Vfs::OpenMode::ReadOnly;
            break;
        case O_WRONLY:
            open_mode = Vfs::OpenMode::WriteOnly;
            break;
        case O_RDWR:
        default:
            open_mode = Vfs::OpenMode::ReadWrite;
            break;
    }

    Vfs::FileDescriptor* vfs_fd = Vfs::open(path, open_mode);
    if (vfs_fd == nullptr) {
        return -EBADF;
    } else if ((flags & O_DIRECTORY) && vfs_fd->node->type != Vfs::NodeType::Directory) {
        return -ENOTDIR;
    }

    int id = task->add_file_handle(vfs_fd);
    *fd = id;

    return 0;
}

int sys_get_cwd(char* buffer, [[gnu::unused]] size_t size) {
    auto task = Scheduler::get_current_task();
    strcpy(buffer, task->working_dir);
    return 0;
}

int sys_close(int fd) {
    auto task = Scheduler::get_current_task();
    Vfs::FileDescriptor* vfs_fd = task->get_file_handle(fd);
    if (vfs_fd == nullptr) {
        return -EBADF;
    }

    task->handles.remove(vfs_fd);
    Vfs::close(vfs_fd);
    return 0;
}

int sys_stat(int fd, const char* path, [[gnu::unused]] int flags, struct stat* statbuf) {
    auto task = Scheduler::get_current_task();

    Vfs::Node* node; 
    if (fd != AT_FDCWD) {
        auto handle = task->get_file_handle(fd);
        if (handle == nullptr) {
            return -EBADF;
        }
        node = handle->node;
    } else {
        char* absolute_path = strdup(task->working_dir);
        strcat(absolute_path, path);
        node = Vfs::get_node(absolute_path);
    }

    // Most of the fields have the value 0
    memset(statbuf, 0, sizeof(statbuf));

    switch (node->type) {
        case Vfs::NodeType::Directory:
            statbuf->st_mode |= S_IFDIR;
            break;
       case Vfs::NodeType::File:
            statbuf->st_mode |= S_IFREG;
            break;
      default:
            __builtin_unreachable();
            break;
    }

    statbuf->st_size = node->file_size;
    statbuf->st_ino = node->inode;

    return 0;
}

int sys_read_entries(int fd, void* buffer, [[gnu::unused]] size_t max_size, size_t* bytes_read) {
    auto task = Scheduler::get_current_task();
    auto handle = task->get_file_handle(fd);

    if (handle == nullptr) {
        return -EBADF;
    } else if (handle->node->type != Vfs::NodeType::Directory) {
        return -ENOTDIR;
    }

    auto child = handle->node->children->get_nth(handle->rw_position);
    if (child.is_none()) {
        *bytes_read = 0;
        return 0;
    }

    Vfs::Node* node = child.unwrap();

    auto entry = (struct dirent*)buffer;
    strcpy(entry->d_name, node->name);
    entry->d_ino = node->inode;
    entry->d_off = 0;
    entry->d_reclen = sizeof(struct dirent);

    switch (node->type) {
        case Vfs::NodeType::Directory:
            entry->d_type = DT_DIR;
            break;
       case Vfs::NodeType::File:
            entry->d_type = DT_REG;
            break;
      default:
            __builtin_unreachable();
            break;
    }

    handle->rw_position++;
    *bytes_read = sizeof(struct dirent);

    return 0;
}

// FIXME: This is horrible. Using nodes would be better. Also, we need to support dots in paths.
int sys_chdir(const char* path) {
    auto task = Scheduler::get_current_task();

    if (path[0] == '/') {
        // Absolute path
        delete task->working_dir;
        task->working_dir = strdup(path);
    } else {
        // Relative path
        strcat(task->working_dir, "/");
        strcat(task->working_dir, path);
    }

    return 0;
}

int sys_mkdir(const char* path, [[gnu::unused]] mode_t mode) {
    Vfs::create(path, Vfs::NodeType::Directory);
    return 0;
}
