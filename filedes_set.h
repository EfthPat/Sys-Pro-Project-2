#ifndef FILEDES_SET_H
#define FILEDES_SET_H

#include <iostream>
#include <sys/select.h>

#define FD_CAP 1023

class filedes_set
{
    protected:

    int max_fd;
    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;
    struct timeval timeout;

    public:
    filedes_set();

    int add_fd_rd(int);
    int del_fd_rd(int);
    int fd_in_rd(int);

    int add_fd_wr(int);
    int del_fd_wr(int);
    int fd_in_wr(int);

    int add_fd_ex(int);
    int del_fd_ex(int);
    int fd_in_ex(int);

    void print_fds(void);
};


#endif