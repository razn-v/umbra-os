#include <kernel/fs/vfs.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/terminal.hpp>

namespace Vfs {

static Node* mountpoints[26];

Node* get_mountpoint(char device) {
    return mountpoints[device - 'A'];
}

void mount(char device, FileSystem* fs) {
    Node* root = new Node;

    // FIXME: I don't like this
    root->name = new char[3];
    root->name[0] = device;
    root->name[1] = ':';
    root->name[2] = '\0';

    root->fs = fs;
    root->type = NodeType::Directory;
    root->parent = nullptr;
    root->childrens = new StringHashMap<Node*, 256>;

    mountpoints[device - 'A'] = root;
    Terminal::printf("Root is %s\n", root->name);
}

// Creates a new directory of file at a given path
Node* create(char device, const char* path, NodeType type) {
    Node* root = mountpoints[device - 'A'];
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
        Optional<Node*> node = parent->childrens ? parent->childrens->get(current_node_name) : 
            Optional<Node*>();
        if (node.is_none()) {
            // Create the new file or directory
            Node* new_node = new Node;

            new_node->name = strdup(current_node_name);
            new_node->fs = root->fs;
            // If we are not at the last node of the path, it can only be a directory
            new_node->type = strcmp(sub_path_copy, "") ? type : NodeType::Directory;
            new_node->parent = parent;

            if (new_node->type == NodeType::Directory) {
                new_node->childrens = new StringHashMap<Node*, 256>;
            }

            // Add the new node to the parent's children.
            parent->childrens->insert(new_node->name, new_node);
            parent = new_node;
        } else {
            parent = node.unwrap();
        }
    }
    
    delete sub_path;
    return parent;
}

FileDescriptor* open(char device, const char* path, OpenMode mode) {
    Node* root = mountpoints[device - 'A'];
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
        Optional<Node*> node = parent->childrens->get(current_node_name);
        if (node.is_none()) {
            delete sub_path;
            return nullptr; 
        } else {
            parent = node.unwrap();
        }
    }

    FileDescriptor* fd = new FileDescriptor; 
    fd->node = parent;
    fd->mode = mode;
    fd->read_position = 0;
    fd->write_position = 0;

    delete sub_path;
    return fd;
}

void close(FileDescriptor* fd) {
    if (fd == nullptr) return;
    delete fd;
}

size_t read(FileDescriptor* fd, void* buffer, size_t length) {
    if (fd == nullptr || fd->read_position + length > fd->node->file_size || 
            fd->mode == OpenMode::WriteOnly) {
        return 0;
    }

    size_t bytes_read = fd->node->fs->read(fd->node, buffer, fd->read_position, length);
    if (fd->read_position + length < fd->node->file_size) {
        fd->read_position += length;
    }

    return bytes_read;
}

size_t write(FileDescriptor* fd, void* buffer, size_t length) {
    if (fd == nullptr || fd->mode == OpenMode::ReadOnly) {
        return 0;
    }

    if (fd->node->file_size == 0) {
        fd->node->file_data = new uint8_t[length];
        fd->node->file_size = length;
        fd->node->capacity = length;
    } else if (fd->write_position + length > fd->node->capacity) {
        // Double the size of the file until we meet the requirements
        size_t new_capacity = fd->node->capacity;
        while (fd->write_position + length > new_capacity) {
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

    size_t bytes_written = fd->node->fs->write(fd->node, buffer, fd->write_position, length);
    if (fd->write_position + length > fd->node->file_size) {
        fd->node->file_size = fd->write_position + length;
    }
    fd->write_position += length;

    return bytes_written;
}

// Reposition read offset
void seek_read(FileDescriptor* fd, size_t offset, SeekMode mode) {
    if (fd == nullptr) return;

    if (mode == SeekMode::Set) {
        fd->read_position = offset;
    } else if (mode == SeekMode::Current) {
        fd->read_position += offset;
    } else if (mode == SeekMode::End) {
        fd->read_position = fd->node->file_size + offset;
    }
}

// Reposition write offset
void seek_write(FileDescriptor* fd, size_t offset, SeekMode mode) {
    if (fd == nullptr) return;

    if (mode == SeekMode::Set) {
        fd->write_position = offset;
    } else if (mode == SeekMode::Current) {
        fd->write_position += offset;
    } else if (mode == SeekMode::End) {
        fd->write_position = fd->node->file_size + offset;
    }
}

// TODO: Remove this and make the fields of hashmap private
void print_tree(Node* root, size_t level) {
    if (root == nullptr) return;

    // Print indentation based on the level
    for (size_t i = 0; i < level; i++) {
        Terminal::printf("   ");
    }

    // Print the node name
    if (root->type == Vfs::NodeType::Directory) {
        Terminal::printf("{yellow}%s/\n", root->name);
    } else {
        Terminal::printf("%s\n", root->name);
    }

    for (size_t i = 0; i < 256; i++) {
        StringHashMap<Node*, 256>::Entry* entry = root->childrens ? root->childrens->entries[i] :
            nullptr;

        while (entry != nullptr) {
            Node* value = entry->value;
            print_tree(value, level + 1);
            entry = entry->next;
        }
    }
}

}
