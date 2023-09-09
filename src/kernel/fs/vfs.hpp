#pragma once

#include <kernel/utils/hashmap.hpp>
#include <stddef.h>
#include <sys/types.h>

namespace Vfs {

enum class SeekMode {
    Set,
    Current,
    End
};

enum class NodeType {
    File,
    Directory
};

enum class OpenMode {
    ReadOnly,
    WriteOnly,
    ReadWrite
};

class FileSystem;

struct Node {
    char* name;
    FileSystem* fs;

    NodeType type;
    Node* parent = nullptr;
    // Only for directories
    StringHashMap<Node*, 256>* children = nullptr;
    // Only for files
    uint8_t* file_data = nullptr;
    size_t file_size = 0;
    // Number of bytes that the file takes in memory, not necessarily filled with data
    size_t capacity = 0;
    size_t inode;
};

struct FileDescriptor {
    int id = -1;
    Node* node;
    OpenMode mode;
    // Read & write position in the file. 
    // For directories, it indicates the index in the directory tree.
    size_t rw_position = 0;

    FileDescriptor() = default;

    FileDescriptor(FileSystem* fs, int id, OpenMode mode, size_t size) : id(id), mode(mode) {
        this->node = new Node; 
        this->node->fs = fs;
        this->node->file_data = new uint8_t[size]; 
        this->node->file_size = size; 
        this->node->capacity = size; 
    }
};

class FileSystem {
public:
    virtual size_t read(Node* node, void* buffer, size_t offset, size_t length) = 0;
    virtual size_t write(Node* node, const void* buffer, size_t offset, size_t length) = 0;
};

Node* get_root();
FileDescriptor* get_stdin();
FileDescriptor* get_stdout();
FileDescriptor* get_stderr();

void mount(Node* node, FileSystem* fs);
void init_std(FileSystem* fs);
Node* create(const char* path, NodeType type);

Node* get_node(const char* path);
FileDescriptor* open(const char* path, OpenMode mode);
void close(FileDescriptor* fd);
ssize_t read(FileDescriptor* fd, void* buffer, size_t length);
ssize_t write(FileDescriptor* fd, const void* buffer, size_t length);
void seek(FileDescriptor* fd, size_t offset, SeekMode mode);

void print_tree(Node* root, size_t level = 0);

}
