
#include "ID.h"

static int IDS[BUFSZ] = {-1}; // key -> ID, value -> fd
static int NOT_USED_IDS[BUFSZ] = {-1};//used to minimize IDs if client closes

static int last_data_fork = 0;// number of active clients
static int last_used_ip = -1;// shows if there is unused IDs less than current

//returns new IP
int AddID(int fd) {
        if (last_used_ip == -1) {
            IDS[last_data_fork] = fd;
            last_data_fork++;
            return last_data_fork - 1;
        } else {
            int free_space = NOT_USED_IDS[last_used_ip];
            IDS[free_space] = fd;
            last_used_ip--;
            return free_space;
        }
}

//returns file descriptor
void DeleteID(int ID) {
    close(IDS[ID]);
    IDS[ID] = -1;

    last_used_ip++;
    NOT_USED_IDS[last_used_ip] = ID;
}

//returns -1 if client wasn't found
int GetFD_FromID(int ID) {
    if (ID < 0 || ID > last_data_fork)
        return -1;

    return IDS[ID];
}

//kills all slaves
void Close_IDS() {
    for (int cur_id = 0; cur_id < last_data_fork; ++cur_id) {
        if (IDS[cur_id] != -1) {
            char buf[] = "CLOSE_SERVER";
            pack_unnamed_t* pack = CreatePack_Unnamed(buf, strlen(buf));
            WritePack_Unnamed(IDS[cur_id], pack);
            DestroyPack_Unnamed(pack);

            close(IDS[cur_id]);
        }
    }
}