//
// Created by iliya on 26.04.2021.
//

#include "../my_server.h"
#include "../packet/packet.h"
#include "../ID/ID_TCP.h"

#define wait_ms 1000

int create_bash ();
int check_bash (int fd);

int SetSlaveLog (char* ID);
int start_slave(int ID);

int check_buffer (char* buffer);
void print_cur_dir ();
int send_message (char* str);
int send_message_bash (char* str);
int do_ls ();

int CheckArgs (int argc , char* argv []);
//dynamic output
char* write_into_bash (int fd , pack_named_t* pack , struct pollfd* pollfds);


static int my_socket = 0;
static int port = 0;
static struct sockaddr* name = NULL;
static pid_t parent_pid = 0;


/* argv:
[1] - socket sending
[2] - sin_port
[3] - sin_addr
[4] - pid of parent
[5] - ID of client to create log file
*/
int main(int argc, char** argv) {

    if (CheckArgs(argc, argv) == -1)
        return -1;
    if (SetSlaveLog(argv[5]) == -1) {
        return -1;
    }
    struct in_addr addr = { atoi (argv[3]) };
    struct sockaddr_in sock_addr = { AF_INET, port, addr, 0 };
    name = (struct sockaddr*)(&sock_addr);

    pr_info ("Server slave was initialized");
    send_message("Ready to use!\n");

    start_slave(atoi(argv[5]));

    pr_info ("Exit program server_slave");
    close(my_socket);
    return 0;
}
int CheckArgs (int argc , char* argv []) {


    if (argc != 6)
    {
        pr_err ("Not enough parameters: %d. Must be 5" , argc);
        return -1;
    }

    my_socket = atoi (argv[1]);
    port = atoi (argv[2]);
    parent_pid = atoi(argv[4]);
    return 0;
}
int SetSlaveLog (char* ID) {
    char buf[100] = {};
    if (sprintf(buf, "/home/iliya/server_logs/slave_tcp%s.log", ID) == -1) {
        pr_strerr("Can't create name of log file slave%s" , ID);
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
    return 0;
}

int start_slave(int ID) {

    int ret = 0;
    pack_named_t* pack = NULL;
    while(1) {
        pack = ReadPack_Named(my_socket, name);
        if (pack == NULL) {
            pr_strerr("Cant read pack");
            DestroyPack_Named(pack);
            ret = -1;
            return ret;
        }

        if (strcmp(pack -> data_, "EXIT") == 0) {break;}
        if (strcmp(pack -> data_, "exit") == 0) {break;}

        if (strcmp(pack -> data_, "bash") == 0) {DestroyPack_Named(pack); return create_bash();}

        if (strcmp(pack -> data_, "CLOSE_SERVER") == 0) {
            delete_ID(ID);
            pr_info("Killing server");
            DestroyPack_Named(pack);
            if (kill (parent_pid , SIGUSR1) == -1) {
                pr_strerr ("Can't kill pid %d" , parent_pid);
                return -1;
            }
            return 0;
        }
        if (check_buffer(pack -> data_) == -1) {ret = -1; break;}
        DestroyPack_Named(pack);

    }
    delete_ID(ID);
    DestroyPack_Named(pack);
    return ret;


}
int send_message (char* str) {
    pr_info ("Sending message");
    size_t size = strlen(str);
    pack_named_t* pack = CreatePack_STATIC(str, size, 0);

    if (pack == NULL) {
        free(pack);
        return -1;
    }
    pr_info("Point #1");
    if (WritePack_Named(my_socket, name, pack) == -1) {
        pr_strerr("Sending message error");
        DestroyPack_Named(pack);
        return -1;
    }
    free(pack);
    pr_info("Message was sent");
    return 0;
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
int create_bash () {

    pr_info ("Creating bash!");
    int fd = open ("/dev/ptmx" , O_RDWR | O_NOCTTY);
    if (fd == -1) {
        pr_strerr("Cant open /dev/ptmx");
        return -1;
    }
    if (grantpt(fd) == -1) { pr_strerr("Cant grantpt bash"); return -1;}
    if (unlockpt(fd) == -1) { pr_strerr("Cant unlockpt bash"); return -1;}

    char* path = ptsname(fd);
    if (path == NULL) { pr_strerr("Cant set ptsname fo %d", fd); return -1;}
    int fd2 = open(path, O_RDWR);
    if (fd2 == -1) { pr_strerr("cant open %s ", path); return -1;}
    struct termios term;
    term.c_lflag = 0;
    if (tcsetattr (fd2 , 0 , &term) == -1) {
        pr_strerr ("Can't set tc_attr in %d fd" , fd2);
        return -1;
    }


    pid_t pd = fork();
    if (pd == 0) {
        if (dup2(fd2, STDIN_FILENO)  == -1) {pr_strerr("STDIN dup error! ");  return -1;}
        if (dup2(fd2, STDOUT_FILENO) == -1) {pr_strerr("STDOUT dup error! "); return -1;}
        if (dup2(fd2, STDERR_FILENO) == -1) {pr_strerr("STDERR dup error! "); return -1;}
        close(fd);

        if (execlp("/bin/bash" , "/bin/bash" , NULL) == -1) { pr_strerr("exec bash problem!"); return -1;}

    }
    pr_info ("Created bash of server slave");

    int ret = check_bash (fd);
    close (fd);
    close (fd2);
    return ret;
}

int PrintFirstCommand (int fd , struct pollfd* poll);
int check_bash(int fd) {

    pr_info("Checking bash");
    if (send_message ("Bash was started!\n") == -1) {return -1;}

    struct pollfd pollfds;
    pollfds.fd = fd;
    pollfds.events = POLLIN;
    /*if (PrintFirstCommand (fd , &pollfds) == -1)
        return -1;*/

    if (PrintFirstCommand (fd , &pollfds) == -1)
        return -1;
    pr_info ("Main checking bash is started!");

    pack_named_t* pack = NULL;

    while(1) {

        pack = ReadPack_Named(my_socket, name);
        pr_info ("Got message: %s" , pack->data_);
        if (strcmp (pack->data_ , "CLOSE_SERVER") == 0) {
            kill(parent_pid, SIGKILL);
            DestroyPack_Named(pack);
            return -1;
        }
        char* buf = write_into_bash(fd, pack, &pollfds);
        if (buf == NULL) { DestroyPack_Named(pack); return -1;}
        DestroyPack_Named(pack);
        pr_info ("SENDING: %s" , buf);
        send_message_bash(buf);

    }


}
int PrintFirstCommand (int fd , struct pollfd* poll) {

    pack_named_t* pack = CreatePack_Named("\n", 1, 0);
    if (pack == NULL)
        return -1;
    char* buf = write_into_bash(fd, pack, poll);
    if (buf == NULL)
        return -1;
    DestroyPack_Named(pack);
    return 0;
}


int check_buffer (char* buffer) {
    pr_info ("Checking buffer: %s" , buffer);
    int err;
    int flag = 0;
    if (strcmp (buffer , "ls") == 0) {
        flag = 1;
        if (do_ls() == -1)
            return -1;
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

    return 0;

}
int do_ls () {
    pr_info ("start oing ls");
    char buffer[BUFSZ];
    if (getcwd(buffer, BUFSZ) == NULL) {pr_strerr("Cant define current working directory!"); return -1;}
    int new_pipe[2] = {};
    if (pipe(new_pipe) == -1) {pr_strerr("Cant create new pipe! "); return -1;}

    pid_t pd = fork();
    if (pd == 0) {
        if(dup2(new_pipe[1], STDOUT_FILENO) == -1) {pr_strerr("Cant dup file %d", new_pipe[1]); return -1;}
        if (execlp("ls", "ls", NULL) == -1) {pr_strerr ("Exec ls wasn't done properly!"); return -1;}

    }
    waitpid(pd, NULL, 0);
    printf("%d %d\n", new_pipe[0], new_pipe[1]);
    if (write(new_pipe[1], "\0", 1) == -1) {pr_strerr("Cant write! "); return -1;}
    char buffer2[BUFSZ] = {};

    if (read(new_pipe[0], buffer2, BUFSZ ) == -1) {pr_strerr("Cant read from pipe! "); return -1;}
    else {
        strcat(buffer, "\n");
        strcat(buffer2, "\0");
        strcat(buffer, buffer2);

        send_message(buffer);

    }
    close(new_pipe[0]);
    close(new_pipe[1]);
    pr_info("Done ");
    return 0;

}

char big_buffer[64 * BUFSZ] = {};
char* write_into_bash (int fd , pack_named_t* pack , struct pollfd* pollfds) {

    memcpy(big_buffer, pack ->data_, pack ->size_);
    big_buffer[pack ->size_] = '\n';
    big_buffer[pack ->size_ + 1] = '\0';

    if (write(fd, big_buffer, pack -> size_ + 2) == 1) {pr_strerr("Cant write into bash! \n"); return NULL;}

    int bytes = 0;

    while (poll(pollfds, 1, wait_ms) != 0) {

        if (pollfds -> revents == POLLIN) {

            int ret = read(fd, big_buffer + bytes, sizeof(big_buffer) - bytes);
            if (ret <= 0) {pr_strerr("Cant read! \n"); return NULL;}
            bytes += ret;

        }

    }
    /*while (bytes > 0 && big_buffer[bytes - 1] != '#')
        bytes--;
    big_buffer[bytes] = ' ';
    big_buffer[bytes + 1] = '\0';
    big_buffer_size = bytes + 2;*/
    big_buffer[bytes] = '\0';
    return big_buffer;


}
int send_message_bash (char* str) {
    pr_info ("Sending message");
    size_t size = strlen (str);

    pack_named_t* packet = CreatePack_STATIC (str , size  + 1, 0);
    if (packet == NULL)
        return -1;

    str[size - 1] = ' ';
    str[size] = '\0';
    int ret = (WritePack_Named (my_socket , name , packet) == -1);
    pr_info("MEssage was sent");

    free(packet);
    return ret;
}

