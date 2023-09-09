#pragma once

#include <stdint.h>

#define USTAR_TYPE_NORMAL     '0'
#define USTAR_TYPE_HARD_LINK  '1'
#define USTAR_TYPE_SYM_LINK   '2'
#define USTAR_TYPE_CHAR_DEV   '3'
#define USTAR_TYPE_BLOCK_DEV  '4'
#define USTAR_TYPE_DIRECTORY  '5'
#define USTAR_TYPE_NAMED_PIPE '6'

namespace Initramfs {

struct [[gnu::packed]] UstarHeader {
    char name[100];
    char file_mode[8];
    char uid[8];
    char guid[8];
    char size[12];
    char mod_time[12];
    char checksum[8];
    char type_flag;
    char link_name[100];
    char magic[6];
    char version[2];
    char owner_name[32];
    char group_name[32];
    char dev_major[8];
    char dev_minor[8];
    char prefix[155];
};

void init();

};
