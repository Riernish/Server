//
// Created by iliya on 27.04.2021.
//

#include "../my_server.h"


static int last_data_fork = 0;// number of active clients
static int last_used_ip = -1;// shows if there is unused IDs less than current
static int IDS[BUFSZ] = {-1}; // key -> ID, value -> fd
static int NOT_USED_IDS[BUFSZ] = {-1};//used to minimize IDs if client closes
int get_new_ID();
void delete_ID(int ID);
