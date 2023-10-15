#include <kernel/fs/vfs.hpp>

class Tty : public Vfs::FileSystem {
public:
    size_t read(Vfs::Node* node, void* buffer, size_t offset, size_t length) override;
    size_t write(Vfs::Node* node, const void* buffer, size_t offset, size_t length) override;
};
