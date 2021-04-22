
#include "my_server.h"
#include "packet/packet.h"
#include "ID/ID.h"

void WriteMessage (int ID , pack_unnamed_t* pack);
char CheckNewClient (pack_named_t* pack , struct sockaddr_in* addr , int sk);
void close_server (int socket);
char start_daemon();

static char slave_barn[100] = {};

int main() {

    if (start_daemon() == -1)
        return EXIT_SUCCESS;
    int sk;
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        ERROR("Unable to create socket");
        exit(EXIT_FAILURE);

    }
    struct in_addr in_ad = { inet_addr (MY_IP )};
    struct sockaddr_in name = { AF_INET, PORT, in_ad };

    struct sockaddr* name_ = (struct sockaddr*)&name;
    socklen_t sock_len = sizeof (struct sockaddr_in);





    if (bind(sk, name_,  sock_len) == -1) {
        close(sk);
        ERROR("Unable to bind socket");
        exit(EXIT_FAILURE);


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
    close_server(sk);


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
    if (fd == -1) { ERROR("Detected -1 pipe"); exit(EXIT_FAILURE); }
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
            exit(EXIT_FAILURE);

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
            ret = execlp(slave_barn, slave_barn, out_str[0], out_str[1], out_str[2], out_str[3], out_str[4], NULL);
        if (pd == 0 && ret == -1) {
            ERROR("Unable to make slave work! ");
            raise(SIGKILL);
        }
        return 1;

    }
    return 0;
}
//tells slaves to close
void close_server (int socket){
    close(socket);
    Close_IDS();
}

char start_daemon() {

    //printf("Initializing daemon!\n");
    char* cur_dir = get_current_dir_name();
    if (cur_dir == NULL) { return -1;}
    //printf("%s\n", cur_dir);
    memcpy (slave_barn , cur_dir , strlen (cur_dir));
    strcat (slave_barn , "/server_slave\0");

    free(cur_dir);

    pid_t pid = fork();
    if (pid == -1) {ERROR("Unable to create daemon\n"); return -1;}

    if (pid != 0) return -1;

    umask(0);
    pid_t sid = setsid();
    if (sid == -1) { ERROR("Cant set sid!\n"); return -1;}
    if (chdir("/") == -1) {ERROR("Cant change directory\n"); return -1;}

    close (STDIN_FILENO);
    close (STDOUT_FILENO);
    close (STDERR_FILENO);


    return 0;
}
