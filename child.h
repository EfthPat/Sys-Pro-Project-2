#ifndef CHILD_H
#define CHILD_H

#include <sys/types.h>
#include <string>

class child
{
    private:

    pid_t pid;
    int read_fd;
    int write_fd;
    unsigned int pr_number;
    std::string dirs;
    std::string** dirs_ptr;

    public:

    // Getters
    pid_t get_pid(void);
    int get_read_fd(void);
    int get_write_fd(void);
    unsigned int get_pr_number(void);
    std::string get_dirs(void);
    std::string** get_dirs_ptr(void);

    // Setters
    void set_pid(pid_t);
    void set_read_fd(int);
    void set_write_fd(int);
    void set_pr_number(unsigned int);
    void set_dirs(std::string);
    void set_dirs_ptr(std::string**);


};


#endif
