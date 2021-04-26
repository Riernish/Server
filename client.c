
#include "my_server.h"
#include "packet/packet.h"
#include "log.h"




in_addr_t check_arg (int argc , char* argv);
int GetID(int sk, struct sockaddr* addr1, struct sockaddr* addr2);
int SetClientLogTmp ();
int SetClientLog (int Client_ID);

void client_routine(int sk, struct sockaddr* sending_, struct sockaddr* receiving_, int Client_ID);

int main(int argc, char** argv) {

        if (SetClientLogTmp() == -1) { pr_strerr("Cant set log"); exit(EXIT_FAILURE);}
        const struct in_addr in_ad = { check_arg(argc, argv[1]) };

        int sk = socket (AF_INET , SOCK_DGRAM , 0);
        if (sk == -1) { pr_strerr("Unable to connect to socket!\n"); exit(EXIT_FAILURE);}
        if (argc == 2 && strcmp (argv[1] , "broadcast") == 0) {
            int yes = 1;
            if (setsockopt(sk, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) == -1) {
                pr_strerr("Can't optionalize socket!");
                exit(EXIT_FAILURE);

            }
        }
        const struct sockaddr_in sending = { AF_INET, PORT, in_ad };
        struct sockaddr* sending_ = (struct sockaddr*)&sending;

        //to get information from server
        ///////////////////////////
        const struct sockaddr_in receiving = { AF_INET, 0, in_ad };
        struct sockaddr* receiving_ = (struct sockaddr*)&receiving;
        socklen_t sock_len = sizeof (struct sockaddr_in);
        if (bind(sk, receiving_, sock_len) == -1) {

            close(sk);
            pr_strerr("Unable to bind socket");
            exit(EXIT_FAILURE);
        }
        /////////////////////////

        pr_info("Client was initialized");
        int Client_ID = -2;
        printf("Connecting to the server...\n");
        Client_ID = GetID(sk, sending_, receiving_);
        if (Client_ID == -2) { pr_strerr("No server found"); exit(EXIT_FAILURE);}
        printf("Connected!\n");

        if (SetClientLog(Client_ID) == -1) { pr_strerr("Cant set log"); exit(EXIT_FAILURE);}

        client_routine(sk, sending_, receiving_, Client_ID);

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

//receives ID from server
int GetID(int sk, struct sockaddr* addr1, struct sockaddr* addr2) {

    char buf[5] = "GETID";
    pack_named_t* pack = CreatePack_Named(buf, 5, -1);
    WritePack_Named(sk, addr1, pack);
    DestroyPack_Named(pack);

    pack_named_t * packet = ReadPack_Named(sk, addr2);
    int out = atoi(packet ->data_);
    printf("Client ID: %d \n", out);

    DestroyPack_Named(packet);

    return out;
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
    strcat (client_log , "/client_logs/client.log\0");
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
int SetClientLog (int Client_ID) {

    pr_info("Setting log file");
    char client_log[100] = {};
    char* cur_dir = get_current_dir_name();
    if (cur_dir == NULL) { return -1;}
    memcpy (client_log, cur_dir , strlen (cur_dir));
    client_log[strlen(cur_dir)] = '\0';
    char buf[100] = {};
    sprintf (buf , "/client_logs/client%d.log", Client_ID);
    strcat(client_log, buf);
    free(cur_dir);

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

void client_routine(int sk, struct sockaddr* sending_, struct sockaddr* receiving_, int Client_ID) {
    char buffer[BUFSZ] = {};

    while (strcmp (buffer , "CLOSE_SERVER") != 0) {

        if (fgets(buffer, BUFSZ, stdin)== NULL) {
            pr_strerr("Ne chitayetsya stroka ");
            exit(EXIT_FAILURE);

        }

        size_t len = strlen (buffer);
        buffer[len - 1] = '\0'; //delete last '\n' after fgets


        pack_named_t *pack = CreatePack_Named(buffer, len, Client_ID);
        WritePack_Named(sk, sending_, pack);
        DestroyPack_Named(pack);

        if (strcmp (buffer , "EXIT") == 0) //EXIT sends to server to kill slave
            break;


        pack_named_t * packet = ReadPack_Named(sk, receiving_);
        pr_info("%s", packet -> data_);
        printf("%s", packet ->data_);
        if (strcmp(packet -> data_, "SERVER_CLOSED!\n") == 0) {DestroyPack_Named(packet); break;}
        DestroyPack_Named(packet);
    }
}