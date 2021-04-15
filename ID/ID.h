#include "../packet/packet.h"
#include "../my_server.h"

#define NEW_CLIENT -1

//returns new IP
int AddID(int fd);

//returns file descriptor
void DeleteID(int ID);

//returns -1 if client wasn't found
int GetFD_FromID(int ID);

void Close_IDS();