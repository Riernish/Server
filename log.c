//
// Created by iliya on 24.04.2021.
//

//
// Created by iliya on 22.04.2021.
//

#include "log.h"

#define BUFSZ 4096
static char BUF[BUFSZ] = {};
static int pos = 0;
static int logfd = STDOUT_FILENO;

int Check_Bytes (int bytes) {

    if (bytes <= 0)
        return -1;
    pos += bytes;
    return 0;
}
int log_get_time() {

    long int ttime = time(NULL);

    int bytes = sprintf(BUF + pos, " %s", ctime(&ttime));
    bytes += -1;

    return Check_Bytes(bytes);
}

int log_get_pid() {
    return Check_Bytes(sprintf(BUF + pos, " PID: %d ", getpid()));
}

int log_get_level(int level) {
    int bytes;
    switch (level) {
        case LOG_ERROR:
            bytes = sprintf (BUF + pos , "ERROR: ");
            break;
        case LOG_INFO:
            bytes = sprintf (BUF + pos , "Info: ");
            break;
        case LOG_WARNING:
            bytes = sprintf (BUF + pos , "Warning: ");
            break;
        default:
            bytes = -1;
    }
    return Check_Bytes(bytes);
}

int WriteLoggingFile(int level, char* message, ...) {

    if (logfd < 0)
        return -1;

    va_list param;
    va_start(param, message);

    log_get_level(level);
    int bytes = vsnprintf(BUF + pos, BUFSZ - pos, message, param);
    if (Check_Bytes(bytes) == -1) return -1;

    BUF[pos] = '\n';
    pos -= -1;
    int out = -1;
    if (write(logfd, BUF, pos) > 0)
        out = 0;
    pos = 0;

    return out;
}
int SetLogFile(int fd) {
    if (fd < 0) {
        pr_strerr("Cant change logging directory to %d! \n", fd);
        return  -1;

    }
    logfd = fd;
    return 0;

}
int UnSetLogFile() {

    if (close(logfd) == -1) {
        pr_strerr("Cant close file %d! \n", logfd);
        return -1;
    }
    logfd = -1;
    return 0;
}
