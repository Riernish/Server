//
// Created by iliya on 27.04.2021.
//
#include "ID_TCP.h"


int get_new_ID() {
    if (last_used_ip == -1) {
        IDS[last_data_fork] = last_data_fork;
        last_data_fork++;
        return last_data_fork - 1;
    }
    else {
        int free_space = NOT_USED_IDS[last_used_ip];
        IDS[free_space] = free_space;
        last_used_ip--;
        return free_space;
    }

}
void delete_ID(int ID) {

    IDS[ID] = -1;

    last_used_ip++;
    NOT_USED_IDS[last_used_ip] = ID;
}