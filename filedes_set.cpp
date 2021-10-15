#include "filedes_set.h"

filedes_set::filedes_set()
{
    max_fd=0;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    timeout.tv_sec=0;
    timeout.tv_usec=0;
}

int filedes_set::add_fd_rd(int fd)
{
    if(fd<0 || fd>FD_CAP)
        return -1;
    FD_SET(fd,&read_fds);
    if(max_fd<=fd)
        max_fd=fd+1;
    return 0;
}

int filedes_set::del_fd_rd(int fd)
{
    if(fd<0 || fd>FD_CAP)
        return -1;
    FD_CLR(fd,&read_fds);
    return 0;
}

int filedes_set::fd_in_rd(int fd)
{
    if(fd<0 || fd>FD_CAP)
        return -1;
    if(FD_ISSET(fd,&read_fds)!=0)
        return 1;
    return 0;
}

int filedes_set::add_fd_wr(int fd)
{
    if(fd<0 || fd>FD_CAP)
        return -1;
    FD_SET(fd,&write_fds);
    if(max_fd<=fd)
        max_fd=fd+1;
    return 0;
}

int filedes_set::del_fd_wr(int fd)
{
    if(fd<0 || fd>FD_CAP)
        return -1;
    FD_CLR(fd,&write_fds);
    return 0;
}

int filedes_set::fd_in_wr(int fd)
{
    if(fd<0 || fd>FD_CAP)
        return -1;
    if(FD_ISSET(fd,&write_fds)!=0)
        return 1;
    return 0;
}

int filedes_set::add_fd_ex(int fd)
{
    if(fd<0 || fd>FD_CAP)
        return -1;
    FD_SET(fd,&except_fds);
    if(max_fd<=fd)
        max_fd=fd+1;
    return 0;
}

int filedes_set::del_fd_ex(int fd)
{
    if(fd<0 || fd>FD_CAP)
        return -1;
    FD_CLR(fd,&except_fds);
    return 0;
}

int filedes_set::fd_in_ex(int fd)
{
    if(fd<0 || fd>FD_CAP)
        return -1;
    if(FD_ISSET(fd,&except_fds)!=0)
        return 1;
    return 0;
}

void filedes_set::print_fds()
{
    std::cout<<std::endl;
    std::cout<<"MAX_FD:"<<max_fd<<std::endl;
    std::cout<<"READ_SET:"<<std::endl;
    for(unsigned int i=0; i<=FD_CAP; i++)
    {
        int r_r=FD_ISSET(i, &read_fds);
        if(r_r!=0)
            std::cout<<"RD_FD:"<<i<<std::endl;
    }
    std::cout<<"WRITE_SET:"<<std::endl;
    for(unsigned int i=0; i<=FD_CAP; i++)
    {
        int r_r=FD_ISSET(i, &write_fds);
        if(r_r!=0)
            std::cout<<"WR_FD:"<<i<<std::endl;
    }
    std::cout<<std::endl;
}
