#pragma once

#include <stddef.h>
#include <kernel/utils/hashmap.hpp>

namespace Vfs {

enum class NodeType {
    File,
    Directory
};

enum class OpenMode {
    ReadOnly,
    WriteOnly,
    ReadWrite
};

enum class SeekMode {
    Set,
    Current,
    End
};

class FileSystem;

struct Node {
    char* name;
    FileSystem* fs;

    NodeType type;
    Node* parent;
    // Only for directories
    StringHashMap<Node*, 256>* childrens = nullptr;
    // Only for files
    uint8_t* file_data = nullptr;
    size_t file_size = 0;
    // Number of bytes that the file takes in memory, not necessarily filled with data
    size_t capacity = 0;
};

struct FileDescriptor {
    Node* node;
    OpenMode mode;
    // Positions in the file
    size_t read_position;
    size_t write_position;
};

class FileSystem {
public:
    virtual size_t read(Node* node, void* buffer, size_t offset, size_t length) = 0;
    virtual size_t write(Node* node, void* buffer, size_t offset, size_t length) = 0;
};

void mount(char device, FileSystem* fs);
Node* get_mountpoint(char device);
Node* create(char device, const char* path, NodeType type);

FileDescriptor* open(char device, const char* path, OpenMode mode);
void close(FileDescriptor* fd);
size_t read(FileDescriptor* fd, void* buffer, size_t length);
size_t write(FileDescriptor* fd, void* buffer, size_t length);
void seek_read(FileDescriptor* fd, size_t offset, SeekMode mode);
void seek_write(FileDescriptor* fd, size_t offset, SeekMode mode);

void print_tree(Node* root, size_t level = 0);

}
