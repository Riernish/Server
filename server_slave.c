
#include "my_server.h"
#include "packet/packet.h"

#define wait_ms 1000

void create_bash ();
void check_bash (int fd);

void check_buffer (char* buffer);
void print_cur_dir ();
void send_message (char* str);
void send_message_bash (char* str);
void do_ls ();

//dynamic output
char* write_into_bash (int fd , pack_unnamed_t* pack , struct pollfd* pollfds);

static int pipe_rd = 0;
static int my_socket = 0;
static struct sockaddr* name = NULL;

int main(int argc, char* argv[]) {

    if (argc < 6){ERROR("Not enough parameter"); exit(EXIT_FAILURE);}

    printf("%d\n", getpid());

    pipe_rd = atoi(argv[1]);
    my_socket = atoi(argv[2]);
    int port = atoi(argv[3]);
    struct in_addr addr = { atoi (argv[4]) };
    struct sockaddr_in sock_addr = { AF_INET, port, addr};
    name = (struct sockaddr*)(&sock_addr);
    send_message(argv[5]);

    while(1) {
        pack_unnamed_t* pack = ReadPack_Unnamed(pipe_rd);

        if (strcmp(pack -> data_, "CLOSE_SERVER") == 0) {DestroyPack_Unnamed(pack); send_message("SERVER_CLOSED!\n"); break;}

        if (strcmp(pack -> data_, "bash") == 0) {DestroyPack_Unnamed(pack); create_bash(); continue;}

        check_buffer(pack -> data_);
        fflush(stdout);
        DestroyPack_Unnamed(pack);

    }

    close(pipe_rd);

    return 0;
}

void check_buffer (char* buffer) {
    int err = 0;
    int flag = 0;
    if (strcmp (buffer , "ls") == 0) {
        flag = 1;
        do_ls();
    } else if (buffer[0] == 'c' && buffer[1] == 'd') {

        if (buffer[2] == ' ') {err = chdir(buffer + 3); flag = 1;}
        else if (buffer[2] == '\0') {err = chdir("/"); flag = 1;}
        else
            err = -1;


        if (err == -1) {
            send_message("Can't do this with directories!\n");
        }
        print_cur_dir();
    }else if(strcmp(buffer, "print") == 0) {
        flag = 1;
        send_message(DUMMY_STR);

    }
    if (!flag)
        send_message("There is no suck command!\n");


}
void do_ls () {
    char buffer[BUFSZ];
    if (getcwd(buffer, BUFSZ) == NULL) {ERROR("Cant define current working directory!"); exit(EXIT_FAILURE);}
    int new_pipe[2] = {};
    if (pipe(new_pipe) == -1) {ERROR("Cant create new pipe! "); exit(EXIT_FAILURE);}

    pid_t pd = fork();
    if (pd == 0) {
        dup2(new_pipe[1], STDOUT_FILENO);
        execlp("ls", "ls", NULL);

    }
    waitpid(pd, NULL, 0);
    printf("%d %d\n", new_pipe[0], new_pipe[1]);
    if (write(new_pipe[1], "\0", 1) == -1) {ERROR("Cant write! "); exit(EXIT_FAILURE);}
    char buffer2[BUFSZ] = {};

    if (read(new_pipe[0], buffer2, BUFSZ ) == -1) {ERROR("Cant read from pipe! "); exit(EXIT_FAILURE);}
    else {
        strcat(buffer, "\n");
        strcat(buffer2, "\0");
        strcat(buffer, buffer2);

        send_message(buffer);

    }
    close(new_pipe[0]);
    close(new_pipe[1]);

}
void send_message (char* str) {
    size_t size = strlen(str);
    pack_named_t* pack = CreatePack_Named(str, size, 0);
    WritePack_Named(my_socket, name, pack);
    DestroyPack_Named(pack);

}
void print_cur_dir () {
    char buffer[BUFSZ] = { 0 };
    if (getcwd (buffer , BUFSZ) == NULL)
        send_message ("NO current directory ajajajajaja \n\0");
    else {
        strcat (buffer , "\n\0");
        send_message (buffer);
    }
}
void create_bash () {

    int fd = open ("/dev/ptmx" , O_RDWR | O_NOCTTY);
    grantpt(fd);
    unlockpt(fd);

    char* path = ptsname(fd);
    int fd2 = open(path, O_RDWR);
    struct termios term;
    term.c_lflag = 0;
    tcsetattr (fd2 , 0 , &term);


    pid_t pd = fork();
    if (pd == 0) {
        if (dup2(fd2, STDIN_FILENO) == -1) {ERROR("STDIN dup error! "); exit(EXIT_FAILURE); }
        if (dup2(fd2, STDOUT_FILENO) == -1) {ERROR("STDOUT dup error! "); exit(EXIT_FAILURE); }
        if (dup2(fd2, STDERR_FILENO) == -1) {ERROR("STDERR dup error! "); exit(EXIT_FAILURE); }
        close(fd);

        execlp("/bin/bash" , "/bin/bash" , NULL);

    }
    send_message ("Bash was started!\n");
    check_bash (fd);
    close (fd);
    close (fd2);

}
void check_bash(int fd) {

    struct pollfd pollfds;
    pollfds.fd = fd;
    pollfds.events = POLLIN;

    pack_unnamed_t* pack = CreatePack_Unnamed("ls", 2);
    char* buf = write_into_bash(fd, pack, &pollfds);
    DestroyPack_Unnamed(pack);

    while(1) {

        pack_unnamed_t* packet = ReadPack_Unnamed(pipe_rd);
        if (strcmp (pack->data_ , "CLOSE_SERVER") == 0) { DestroyPack_Unnamed (pack); break; }
        buf = write_into_bash(fd, packet, &pollfds);

        send_message_bash(buf);

    }

}

void send_message_bash (char* str) {
    size_t size = strlen (str);

    pack_named_t* packet = CreatePack_STATIC (str , size  + 1, 0);
    str[size - 1] = ' ';
    str[size] = '\0';
    WritePack_Named (my_socket , name , packet);
    free(packet);
}

char big_buffer[64 * BUFSZ] = {};
char* write_into_bash (int fd , pack_unnamed_t* pack , struct pollfd* pollfds) {

    memcpy(big_buffer, pack ->data_, pack ->size_);
    big_buffer[pack ->size_] = '\n';
    big_buffer[pack ->size_ + 1] = '\0';

    if (write(fd, big_buffer, pack -> size_ + 2) == 1) {ERROR("Cant write into bash! \n"); return NULL;}

    int bytes = 0;

    while (poll(pollfds, 1, wait_ms) != 0) {

        if (pollfds -> revents == POLLIN) {

            int ret = read(fd, big_buffer + bytes, sizeof(big_buffer) - bytes);
            if (ret == 0) {ERROR("Cant read! \n"); return NULL;}
            bytes += ret;

        }

    }
    big_buffer[bytes] = '\0';
    return big_buffer;


}