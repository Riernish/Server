
#include "packet.h"

//!Creating
/////////////////////////////////////////////////
pack_named_t* CreatePack_Named (const char* data , size_t size , int name)
{
    pack_named_t* out = (pack_named_t*)malloc (sizeof (pack_named_t));
    if (out == NULL) {
        free (out);
        ERROR("Can't get dynamic memory for named_pack!\n");

        return NULL;
    }

    out->size_ = size + 1;
    out->name_ = name;
    out->data_ = (char*)malloc (sizeof (char) * out -> size_);

    for (size_t i = 0; i < size; ++i)
        out->data_[i] = data[i];
    out->data_[size] = '\0';

    return out;
}
pack_unnamed_t* CreatePack_Unnamed (const char* data , size_t size)
{
    pack_unnamed_t* out = (pack_unnamed_t*)malloc (sizeof (pack_unnamed_t));
    if (out == NULL) {
        free (out);
        ERROR ("Can't get dynamic memory for unnamed_pack!\n");

        return NULL;
    }

    out->size_ = size;
    out->data_ = (char*)malloc (sizeof (char) * size);
    for (size_t i = 0; i < size; ++i)
        out->data_[i] = data[i];
    out->data_[size] = '\0';

    return out;
}
void DestroyPack_Named (pack_named_t* packet) {
    free (packet->data_);
    free (packet);
}
void DestroyPack_Unnamed (pack_unnamed_t* packet){
    free (packet->data_);
    free (packet);
}

pack_unnamed_t* RecoverPack (pack_named_t* pack) {
    pack_unnamed_t* out = (pack_unnamed_t*)malloc (sizeof (pack_unnamed_t));
    if (out == NULL) {
        free (out);
        ERROR("Can't get dynamic memory for unnamed_pack!\n");

        return NULL;
    }

    out->size_ = pack->size_;
    out->data_ = pack->data_;
    return out;
}
/////////////////////////////////////////////////

//!Actions
/////////////////////////////////////////////////
pack_unnamed_t* ReadPack_Unnamed (int fd) {
    pack_unnamed_t* out = (pack_unnamed_t*)malloc (sizeof (pack_unnamed_t));
    if (out == NULL) {
        free (out);
        ERROR("Can't get dynamic memory for unnamed_pack!\n");

        return NULL;
    }

    if (read (fd , &(out->size_) , sizeof (size_t)) == -1) {
        free (out);
        ERROR("Can't properly read size\n");

        return NULL;
    }

    out->data_ = (char*)malloc (out->size_ * sizeof (char));
    if (out->data_ == NULL) {
        free (out);
        ERROR("Can't get dynamic memory for info in pack!\n");
        return NULL;
    }

    if (read (fd , out->data_ , out->size_) == -1) {
        DestroyPack_Unnamed (out);
        ERROR("Can't properly read buffer\n");

        return NULL;
    }

    return out;
}

int WritePack_Unnamed (int fd , pack_unnamed_t* pack) {
    if (pack == NULL) {
        ERROR("Pack is empty!\n");
        return -1;
    }

    if (write (fd , &(pack->size_) , sizeof (size_t)) == -1) {
        ERROR("Can't properly write size!\n");
        return -1;
    }

    if (write (fd , pack->data_ , pack->size_) == -1) {
        ERROR("Can't properly write buffer!\n");
        return -1;
    }

    return 0;
}

pack_named_t* ReadPack_Named (int fd , struct sockaddr* addr) {
    pack_named_t* out = (pack_named_t*)malloc (sizeof (pack_named_t));
    if (out == NULL) {
        free (out);
        ERROR("Can't get dynamic memory for named_pack!\n");

        return NULL;
    }

    socklen_t sock_len = sizeof (struct sockaddr_in);
    if (recvfrom (fd , &(out->name_) , sizeof (out->name_) , 0 , addr , &sock_len) == -1) {
        free (out);
        ERROR("Can't read name from socket!\n");

        return NULL;
    }
    if (recvfrom (fd , &(out->size_) , sizeof (out->size_) , 0 , addr , &sock_len) == -1) {
        free (out);
        ERROR("Can't read size form socket!\n");

        return NULL;
    }

    out->data_ = (char*)malloc (out->size_ * sizeof (char));
    if (out->data_ == NULL) {
        free (out);
        ERROR("Can't get dynamic memory for info in pack!\n");

        return NULL;
    }
    if (recvfrom (fd , out->data_ , out->size_ * sizeof (char) , 0 , addr , &sock_len) == -1) {
        DestroyPack_Named (out);
        ERROR("Can't read data form socket!\n");

        return NULL;
    }

    return out;
}

int WritePack_Named (int fd , struct sockaddr* addr , pack_named_t* pack) {
    if (pack == NULL) {
        ERROR("Pack is empty!\n");
        return -1;
    }

    socklen_t sock_len = sizeof (struct sockaddr_in);
    if (sendto (fd , &(pack->name_) , sizeof (pack->name_) , 0 , addr , sock_len) == -1) {
        ERROR("Can't properly write name!\n");
        return -1;
    }

    if (sendto (fd , &(pack->size_) , sizeof (pack->size_) , 0 , addr , sock_len) == -1) {
        ERROR("Can't properly write size!\n");
        return -1;
    }

    if (sendto (fd , pack->data_ , pack->size_ * sizeof (char) , 0 , addr , sock_len) == -1) {
        ERROR("Can't properly write buffer!\n");
        return -1;
    }

    return 0;
}

pack_named_t* CreatePack_STATIC(char* data, size_t size, int name) {
    pack_named_t* out = (pack_named_t*)malloc (sizeof (pack_named_t));
    if (out == NULL) {
        free (out);
        ERROR("Can't get dynamic memory for named_pack!\n");

        return NULL;
    }

    out->size_ = size;
    out->name_ = name;
    out->data_ = data;
    out->data_[size - 1] = '\0';
    return out;
}
/////////////////////////////////////////////////
