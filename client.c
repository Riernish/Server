
#include "my_server.h"
#include "packet/packet.h"




in_addr_t check_arg (int argc , char* argv);
int GetID(int sk, struct sockaddr* addr1, struct sockaddr* addr2);

int main(int argc, char** argv) {

        const struct in_addr in_ad = { check_arg(argc, argv[1]) };

        int sk = socket (AF_INET , SOCK_DGRAM , 0);
        if (sk == -1) { ERROR("Unable to connect to socket!\n");}
        if (argc == 2 && strcmp (argv[1] , "broadcast") == 0) {
            int yes = 1;
            if (setsockopt(sk, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) == -1) {
                ERROR("Can't optionalize socket!");

            }
        }
        const struct sockaddr_in sending = { AF_INET, PORT, in_ad };
        struct sockaddr* sending_ = (struct sockaddr*)&sending;


        const struct sockaddr_in receiving = { AF_INET, 0, in_ad };
        struct sockaddr* receiving_ = (struct sockaddr*)&receiving;
        socklen_t sock_len = sizeof (struct sockaddr_in);
        if (bind(sk, receiving_, sock_len) == -1) {

            close(sk);
            ERROR("Unable to bind socket");
        }

        int Client_ID = GetID(sk, sending_, receiving_);


        char buffer[BUFSZ] = {};

        while (strcmp (buffer , "CLOSE_SERVER") != 0) {

            if (fgets(buffer, BUFSZ, stdin)== NULL) {
                ERROR("Ne chitayetsya stroka ");

            }

            size_t len = strlen (buffer);
            buffer[len - 1] = '\0'; //delete last '\n' after fgets


            pack_named_t *pack = CreatePack_Named(buffer, len, Client_ID);
            WritePack_Named(sk, sending_, pack);
            DestroyPack_Named(pack);

            if (strcmp (buffer , "EXIT") == 0) //EXIT sends to server to kill slave
                break;


            pack_named_t * packet = ReadPack_Named(sk, receiving_);
            printf("%s", packet ->data_);
            if (strcmp(packet -> data_, "SERVER_CLOSED!\n") == 0) {DestroyPack_Named(packet); break;}
            DestroyPack_Named(packet);
        }

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
        ERROR("Should be 1 or 2 arguments");
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