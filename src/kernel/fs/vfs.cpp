#include <kernel/fs/vfs.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/terminal.hpp>
#include <kernel/dev/tty.hpp>

namespace Vfs {

static Node* root_mountpoint = nullptr;
static size_t next_inode = 0;

// Every task has the same stdin, stdout and stderr for now
static FileDescriptor* stdin;
static FileDescriptor* stdout;
static FileDescriptor* stderr;

FileDescriptor* get_stdin() { return stdin; }
FileDescriptor* get_stdout() { return stdout; }
FileDescriptor* get_stderr() { return stderr; }

Node* get_root() {
    return root_mountpoint;
}

void mount(Node* node, FileSystem* fs) {
    if (node == nullptr) {
        node = new Node;
        node->name = strdup("/");
        node->parent = nullptr;
        root_mountpoint = node;
    }

    node->fs = fs;
    node->type = NodeType::Directory;
    node->children = new StringHashMap<Node*, 256>;
    node->inode = next_inode++;
}

void init_std(FileSystem* fs) {
    stdin = new FileDescriptor(fs, 0, OpenMode::ReadOnly, 4096);
    stdout = new FileDescriptor(fs, 1, OpenMode::WriteOnly, 0);
    stderr = new FileDescriptor(fs, 2, OpenMode::WriteOnly, 0);
}

// Creates a new directory of file at a given path
Node* create(const char* path, NodeType type) {
    Node* root = root_mountpoint;
    if (!root) {
        return nullptr;
    }

    Node* parent = root;
    char* sub_path = strdup(path);
    char* sub_path_copy = sub_path;

    // Go through each directory of the path
    while (*sub_path_copy != '\0') {
        char* current_node_name = sub_path_copy;

        while (*sub_path_copy != '\0' && *sub_path_copy != '/') sub_path_copy++;
        while (*sub_path_copy == '/') *sub_path_copy++ = 0;

        // Check if the current file or directory exists
        Optional<Node*> node = parent->children ? parent->children->get(current_node_name) : 
            Optional<Node*>();
        if (node.is_none()) {
            // Create the new file or directory
            Node* new_node = new Node;

            new_node->name = strdup(current_node_name);
            new_node->fs = root->fs;
            // If we are not at the last node of the path, it can only be a directory
            new_node->type = strcmp(sub_path_copy, "") ? type : NodeType::Directory;
            new_node->parent = parent;
            new_node->inode = next_inode++;

            if (new_node->type == NodeType::Directory) {
                new_node->children = new StringHashMap<Node*, 256>;
            }

            // Add the new node to the parent's children.
            parent->children->insert(new_node->name, new_node);
            parent = new_node;
        } else {
            parent = node.unwrap();
        }
    }
    
    delete sub_path;
    return parent;
}

// NOTE: `path` must be absolute
Node* get_node(const char* path) {
    Node* root = root_mountpoint;
    if (!root) {
        return nullptr;
    }
    
    Node* parent = root;
    char* sub_path = strdup(path);
    char* sub_path_addr = sub_path;

    // Ignore leading slashes
    while (*sub_path == '/') sub_path++;

    // Go through each directory of the path
    while (*sub_path != '\0') {
        char* current_node_name = sub_path;

        while (*sub_path != '\0' && *sub_path != '/') sub_path++;
        while (*sub_path == '/') *sub_path++ = 0;

        // Check if the current file or directory exists
        Optional<Node*> node = parent->children->get(current_node_name);
        if (node.is_none()) {
            delete sub_path_addr;
            return nullptr; 
        } else {
            parent = node.unwrap();
        }
    }

    delete sub_path_addr;
    return parent;
}

FileDescriptor* open(const char* path, OpenMode mode) {
    Node* root = root_mountpoint;
    if (!root) {
        return nullptr;
    }

    Node* parent = Vfs::get_node(path); 
    if (parent == nullptr) {
        return nullptr;
    }

    FileDescriptor* fd = new FileDescriptor; 
    fd->node = parent;
    fd->mode = mode;
    fd->rw_position = 0;

    return fd;
}

void close(FileDescriptor* fd) {
    if (fd == nullptr) return;
    delete fd;
}

ssize_t read(FileDescriptor* fd, void* buffer, size_t length) {
    if (fd == nullptr || fd->rw_position >= fd->node->file_size || 
            fd->mode == OpenMode::WriteOnly || fd->node->type == NodeType::Directory) {
        return -1;
    }

    size_t bytes_read = fd->node->fs->read(fd->node, buffer, fd->rw_position, length);
    if (fd != stdin) {
        fd->rw_position += length;
    }

    return bytes_read;
}

ssize_t write(FileDescriptor* fd, const void* buffer, size_t length) {
    if (fd == nullptr || fd->mode == OpenMode::ReadOnly || fd->node->type == NodeType::Directory) {
        return 0;
    }

    if (fd->node->file_size == 0) {
        fd->node->file_data = new uint8_t[length];
        fd->node->file_size = length;
        fd->node->capacity = length;
    } else if (fd->rw_position + length > fd->node->capacity) {
        // Double the size of the file until we meet the requirements
        size_t new_capacity = fd->node->capacity;
        while (fd->rw_position + length > new_capacity) {
            new_capacity *= 2;
        }

        // Ugly and stupid realloc
        delete fd->node->file_data; 
        uint8_t* data = new uint8_t[new_capacity];
        if (data == nullptr) {
            return -1;
        }

        fd->node->file_data = data;
        fd->node->capacity = new_capacity;
    }

    size_t bytes_written = fd->node->fs->write(fd->node, buffer, fd->rw_position, length);
    if (fd->rw_position + length > fd->node->file_size) {
        fd->node->file_size = fd->rw_position + length;
    }
    fd->rw_position += length;

    return bytes_written;
}

void seek(FileDescriptor* fd, size_t offset, SeekMode mode) {
    if (fd == nullptr) return;

    if (mode == SeekMode::Set) {
        fd->rw_position = offset;
    } else if (mode == SeekMode::Current) {
        fd->rw_position += offset;
    } else if (mode == SeekMode::End) {
        fd->rw_position = fd->node->file_size + offset;
    }
}

}
