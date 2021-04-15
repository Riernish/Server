#pragma once
#include "../my_server.h"

typedef struct
{
    int name_;
    size_t size_;
    char* data_;
} pack_named_t;

typedef struct
{
    size_t size_;
    char* data_;
} pack_unnamed_t;

pack_named_t* CreatePack_Named(const char* data, size_t size, int name);
pack_unnamed_t* CreatePack_Unnamed(const char* data, size_t size);
void DestroyPack_Named(pack_named_t* packet);
void DestroyPack_Unnamed(pack_unnamed_t* packet);

pack_unnamed_t* RecoverPack(pack_named_t* pack);



//!returns dynamic memory
pack_unnamed_t* ReadPack_Unnamed(int fd);
//!return -1 if error
int WritePack_Unnamed(int fd, pack_unnamed_t* pack);



//!returns dynamic memory
pack_named_t* ReadPack_Named(int fd, struct sockaddr* addr);
//!return -1 if error
int WritePack_Named(int fd, struct sockaddr* addr, pack_named_t* pack);

pack_named_t* CreatePack_STATIC(char* data, size_t size, int name);