
#include "my_server.h"
#include "packet/packet.h"
#include "ID/ID.h"

void WriteMessage (int ID , pack_unnamed_t* pack);
char CheckNewClient (pack_named_t* pack , struct sockaddr_in* addr , int sk);
void Close (int socket);

int main() {
    int sk;

    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        ERROR("Unable to create socket");
        return 1;
    }
    struct in_addr in_ad = { inet_addr (MY_IP )};
    struct sockaddr_in name = { AF_INET, PORT, in_ad };

    struct sockaddr* name_ = (struct sockaddr*)&name;
    socklen_t sock_len = sizeof (struct sockaddr_in);





    if (bind(sk, name_,  sock_len) == -1) {
        close(sk);
        ERROR("Unable to bind socket");

        return 1;
    }

    while(1) {

        pack_named_t *pack = ReadPack_Named(sk, name_);
        if (pack == NULL)
            break;
        if (strcmp(pack -> data_, "CLOSE_SERVER") == 0) {
            break;
        }
        if (CheckNewClient(pack, &name, sk) == 0)
            WriteMessage(pack -> name_, RecoverPack(pack));

        DestroyPack_Named(pack);
    }
    Close(sk);


    return 0;
}

////////////////////////////////////////////
//if client closes also kills slave
//if not sends command to slave
////////////////////////////////////////////
void WriteMessage (int ID , pack_unnamed_t* pack) {
    if (strcmp(pack -> data_, "EXIT") == 0) {
        char buf[] = "CLOSE_SERVER";
        pack_unnamed_t* packet = CreatePack_Unnamed(buf, strlen(buf));
        WriteMessage(ID, packet);
        DestroyPack_Unnamed(packet);

        DeleteID(ID);
        return;
    }

    int fd = GetFD_FromID(ID);
    if (fd == -1) { ERROR("Detected -1 pipe"); return; }
    WritePack_Unnamed(fd, pack);
}

////////////////////////////////////////////
//if no such client gets ID and starts slves
//returns 0 otherwise
////////////////////////////////////////////
char CheckNewClient (pack_named_t* pack , struct sockaddr_in* addr , int sk) {

    if (pack ->name_ == NEW_CLIENT) {
        int new_pipe[2] = {};
        if (pipe(new_pipe) == -1) {
            ERROR("Can't create pipe!");

        }
        int new_ID = AddID(new_pipe[1]);
        char out_str[5][16] = {};
        sprintf (out_str[0] , "%d" , new_pipe[0]);
        sprintf (out_str[1] , "%d" , sk);
        sprintf (out_str[2] , "%d" , addr->sin_port);
        sprintf (out_str[3] , "%d" , addr->sin_addr.s_addr);
        sprintf (out_str[4] , "%d" , new_ID);

        pid_t pd = fork();
        int ret;
        if (pd == 0)
            ret = execlp("./server_slave", "./server_slave", out_str[0], out_str[1], out_str[2], out_str[3], out_str[4], NULL);
        if (pd == 0 && ret == -1) {
            ERROR("Unable to make slave work! ");
            raise(SIGKILL);
        }
        return 1;

    }
    return 0;
}
//tells slaves to close
void Close (int socket){
    close(socket);
    Close_IDS();
}
