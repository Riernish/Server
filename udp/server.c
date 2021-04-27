
#include "../my_server.h"
#include "../packet/packet.h"
#include "../ID/ID.h"




int WriteMessage (int ID , pack_unnamed_t* pack);
char CheckNewClient (pack_named_t* pack , struct sockaddr_in* addr , int sk);
void close_server (int socket);
char start_daemon();

static char slave_barn[100] = {};
static const char LOG_FILE [] = "/home/iliya/server_logs/server.log";

int main() {


    char* cur_dir = get_current_dir_name();
    if (cur_dir == NULL) { return -1;}
    memcpy (slave_barn , cur_dir , strlen (cur_dir));
    slave_barn[strlen(cur_dir)] = '\0';
    strcat (slave_barn , "/server_slave\0");
    free(cur_dir);

    if (start_daemon() == -1)
        return EXIT_SUCCESS;

    int sk;
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        pr_err("Unable to create socket");
        exit(EXIT_FAILURE);

    }
    struct in_addr in_ad = { inet_addr (MY_IP )};
    struct sockaddr_in name = { AF_INET, PORT, in_ad };

    struct sockaddr* name_ = (struct sockaddr*)&name;
    socklen_t sock_len = sizeof (struct sockaddr_in);





    if (bind(sk, name_,  sock_len) == -1) {
        close(sk);
        pr_err("Unable to bind socket");
        exit(EXIT_FAILURE);


    }
    pr_info ("Server was initialized!");
    while(1) {

        pack_named_t *pack = ReadPack_Named(sk, name_);
        if (pack == NULL) {
            DestroyPack_Named(pack);
            break;
        }
        if (strcmp(pack -> data_, "CLOSE_SERVER") == 0) {
            DestroyPack_Named(pack);
            break;
        }
        if (CheckNewClient(pack, &name, sk) == 0) {

            pack_unnamed_t *ret = RecoverPack(pack);
            WriteMessage(pack->name_, ret);
            free(ret);
        }

        DestroyPack_Named(pack);
    }
    pr_info ("Exit program");
    close_server(sk);


    return 0;
}

////////////////////////////////////////////
//if client closes also kills slave
//if not sends command to slave
////////////////////////////////////////////
int WriteMessage (int ID , pack_unnamed_t* pack) {
    if (strcmp(pack -> data_, "EXIT") == 0) {
        pr_info ("Client %d has went out!" , ID);
        char buf[] = "CLOSE_SERVER";
        pack_unnamed_t* packet = CreatePack_Unnamed(buf, strlen(buf));
        if (WriteMessage(ID, packet) == -1)
            return -1 ;
        DestroyPack_Unnamed(packet);

        DeleteID(ID);

        return 0;
    }
    pr_info ("Writing message to slave: %s" , pack->data_);

    int fd = GetFD_FromID(ID);
    if (fd == -1) { pr_strerr("Detected -1 pipe"); return -1; }
    if (WritePack_Unnamed(fd, pack) == -1)
        return -1;
    return 0;
}

////////////////////////////////////////////
//if no such client gets ID and starts slves
//returns 0 otherwise
////////////////////////////////////////////
char CheckNewClient (pack_named_t* pack , struct sockaddr_in* addr , int sk) {

    if (pack ->name_ == NEW_CLIENT) {
        int new_pipe[2] = {};
        if (pipe(new_pipe) == -1) {
            pr_strerr("Can't create pipe!");
            exit(EXIT_FAILURE);

        }
        int new_ID = AddID(new_pipe[1]);
        char out_str[5][16] = {};
        pid_t pd = fork();
        if (pd == 0) {
            if (sprintf(out_str[0], "%d", new_pipe[0]) <= 0
                || sprintf(out_str[1], "%d", sk) <= 0
                || sprintf(out_str[2], "%d", addr->sin_port) <= 0
                || sprintf(out_str[3], "%d", addr->sin_addr.s_addr) <= 0
                || sprintf(out_str[4], "%d", new_ID) <= 0) {
                pr_err ("Can't create strings for slaves!");
                return -1;
            }




            int ret = execlp(slave_barn, slave_barn, out_str[0], out_str[1], out_str[2], out_str[3], out_str[4], NULL);
            if (ret == -1) {
                pr_strerr("Unable to make slave work! ");
                raise(SIGKILL);
            }
        }
        return 1;

        pr_info("Created new client!");
    }

    return 0;
}
//tells slaves to close
void close_server (int socket){

    Close_IDS();
    close(socket);


    pr_info ("Unlinked!");
    UnSetLogFile ();
}

char start_daemon() {

    //printf("Initializing daemon!\n");
    pr_info ("Initializing daemon!");


    pid_t pid = fork();
    if (pid == -1) {pr_strerr("Unable to create daemon"); return -1;}

    if (pid != 0) return -1;

    umask(0);
    pid_t sid = setsid();
    if (sid == -1) { pr_strerr("Cant set sid!"); return -1;}
    if (chdir("/") == -1) {pr_strerr("Cant change directory"); return -1;}

    pr_info ("Daemon was initialized!");

    int logfd = open(LOG_FILE, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (logfd == -1) {
        pr_strerr("Can't open file %s! ", LOG_FILE);
        return -1;
    }
    if (SetLogFile(logfd) == -1) {
        return -1;
    }

    close (STDIN_FILENO);
    close (STDOUT_FILENO);
    close (STDERR_FILENO);


    return 0;
}