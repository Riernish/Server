
#include "../my_server.h"
#include "../packet/packet.h"


in_addr_t check_arg (int argc , char* argv);
int GetID(int sk, struct sockaddr* addr1);
int SetClientLogTmp ();
int SetClientLog (int Client_ID);

void client_tcp_routine(int sk, struct sockaddr* sending_);

int main(int argc, char** argv) {

    if (SetClientLogTmp() == -1) { pr_strerr("Cant set log"); exit(EXIT_FAILURE);}
    pr_info("Logging started\n");

    const struct in_addr in_ad = { check_arg(argc, argv[1]) };

    int sk = socket (AF_INET , SOCK_STREAM , 0);
    if (sk == -1) {
        pr_strerr ("Unable to connect to socket!");
        return 1;
    }

    const struct sockaddr_in sending = { AF_INET, PORT, in_ad, 0 };
    struct sockaddr* sending_ = (struct sockaddr*)&sending;


    pr_info("Client was initialized");

    client_tcp_routine(sk, sending_);

    pr_info("Unlinked");
    close (sk);
    return 0;
}
////////////////////////////////////////////////////
//no parameter - self usage
//broadcast - send to all servers
//ip address - connects to ip
////////////////////////////////////////////////////
in_addr_t check_arg (int argc , char* argv) {

    if (argc < 2)
        return inet_addr(MY_ADDRESS);
    if (argc == 2) {
        if (strcmp(argv, "broadcast") == 0)
            return 0;
        return inet_addr(argv);

    }
    if (argc > 2) {
        pr_strerr("Should be 1 or 2 arguments");
        exit(EXIT_FAILURE);
    }
    return -1;

}
void client_tcp_routine(int sk, struct sockaddr* sending_) {

    char buffer[BUFSZ] = {};
    pr_info("Reached point 1");

    while (strcmp (buffer , "CLOSE_SERVER") != 0) {
        if (connect(sk, sending_, sizeof(*sending_)) == -1) {
            pr_strerr("Unable to connect to socket");
            exit(EXIT_FAILURE);
        }

        if (fgets(buffer, BUFSZ, stdin)== NULL) {
            pr_strerr("Cant read string");
            exit(EXIT_FAILURE);

        }

        size_t len = strlen (buffer);
        buffer[len - 1] = '\0'; //delete last '\n'


        pack_named_t *pack = CreatePack_STATIC(buffer, len, 0);
        if (pack == NULL) {
            pr_strerr("Cant create pack");
            return ;
        }
        pr_info ("Client's message: %s" , buffer);
        if (WritePack_Named(sk, sending_, pack) == -1) {
            pr_strerr("Cant write pack ");
            free(pack);
            return;
        }
        pr_info("Reached point 2");
        if (strcmp (buffer , "EXIT") == 0) //EXIT sends to server to kill slave
            break;


        pr_info("Reached point 2.5");
        pack_named_t * packet = ReadPack_Named(sk, sending_);
        pr_info("Reached point 3");

        if (packet == NULL) {
            pr_strerr("Packet is empty");
            exit(EXIT_FAILURE);
        }
        pr_info("Received %s", packet -> data_);
        printf("%s", packet ->data_);
        if (strcmp(packet -> data_, "SERVER_CLOSED!\n") == 0) {DestroyPack_Named(packet); break;}
        DestroyPack_Named(packet);
        pr_info("Reached point 4");
    }
}


int SetClientLogTmp () {

    pr_info("Setting log file");
    char client_log[100] = {};
    char* cur_dir = get_current_dir_name();
    if (cur_dir == NULL) { return -1;}
    memcpy (client_log, cur_dir , strlen (cur_dir));
    client_log[strlen(cur_dir)] = '\0';
    if (mkdir("client_logs", 0777) == -1) {
        if (errno != EEXIST) {
            pr_strerr("Cant make directory client_logs");
            exit(EXIT_FAILURE);
        }
    }
    strcat (client_log , "/client_logs/client_tcp.log\0");
    free(cur_dir);
    char buf[100] = {};
    if (sprintf(buf, "%s", client_log) == -1) {
        pr_strerr("Can't create name of log file client");
        return -1;
    }
    int logfd = open(buf, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (logfd == -1) {
        pr_strerr("Can't open file %s! ", buf);
        return -1;
    }
    if (SetLogFile(logfd) == -1) {
        return -1;
    }
    pr_info ("Log file was successfully set");
    return 0;
}


