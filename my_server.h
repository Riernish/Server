#ifndef _myserver_h
#define _myserver_h

#define _GNU_SOURCE

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>




#define PATH "/tmp/mysock"
#define DUMMY_STR "London is the capital of Great Britain\n"
#define BUFSZ 256
#define MY_ADDRESS "127.0.0.1"
#define MY_IP "127.0.0.1"
#define PORT 23456

#include "error/Error.h"
#include "log/log.h"



#endif
