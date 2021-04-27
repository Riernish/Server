
#include "../my_server.h"
#include "../ID/ID_TCP.h"

void close_server (int socket);
char start_daemon();
void server_tcp_routine(int, struct sockaddr_in*);
int CreateNewClient (int client_sk , struct sockaddr_in* addr);

static char slave_barn[100] = {};
static const char LOG_FILE [] = "/home/iliya/server_logs/server_tcp.log";
static const int SIZE_CONNECTION = 20;

int main() {


    char* cur_dir = get_current_dir_name();
    if (cur_dir == NULL) { return -1;}
    memcpy (slave_barn , cur_dir , strlen (cur_dir));
    slave_barn[strlen(cur_dir)] = '\0';
    strcat (slave_barn , "/server_slave_tcp\0");
    free(cur_dir);

    if (start_daemon() == -1)
        return EXIT_SUCCESS;

    int sk;
    sk = socket(AF_INET, SOCK_STREAM, 0);
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

    server_tcp_routine(sk, &name);

    pr_info ("Exit program");
    close_server(sk);


    return 0;
}


void server_tcp_routine(int sk, struct sockaddr_in* name_) {

    if (listen (sk , SIZE_CONNECTION) == -1) {
        pr_strerr ("Can't listen to sockets");
        return ;
    }

    int client_sk;
    while(1) {

        client_sk = accept (sk , NULL , NULL);
        if (client_sk < 0) {
            pr_strerr ("Can't accept client!");
            break;
        }
        if (CreateNewClient(client_sk, name_) == -1)
            break;
        close(client_sk);
    }

}
int CreateNewClient (int client_sk , struct sockaddr_in* addr) {

    pr_info ("Creating new client at %d fd" , client_sk);

    int cur_pid = getpid ();
    pr_info ("Current pid is %d" , cur_pid);
    char out_str[5][16] = {};
    int new_ID = get_new_ID();
    pid_t pd = fork();
    if (pd == 0) {
        if (sprintf(out_str[0], "%d", client_sk) <= 0
            || sprintf(out_str[1], "%d", addr->sin_port) <= 0
            || sprintf(out_str[2], "%d", addr->sin_addr.s_addr) <= 0
            || sprintf(out_str[3], "%d", cur_pid) <= 0
            || sprintf(out_str[4], "%d", new_ID) <= 0) {
            pr_err ("Can't create strings for slaves!");
            return -1;
        }
        pr_info("Before exec");
        int ret = execlp(slave_barn, slave_barn, out_str[0], out_str[1], out_str[2], out_str[3], out_str[4], NULL);
        pr_info("After exec");
        if (ret == -1) {
            pr_strerr("Unable to make slave work! ");
            return -1;
        }
    }
    pr_info("Client %d created", new_ID);
    return 0;
}

//tells slaves to close
void close_server (int socket){

    //Close_IDS();
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


