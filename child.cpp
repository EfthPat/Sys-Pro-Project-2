#include "child.h"

// Getters
pid_t child::get_pid() {return pid;}
int child::get_read_fd() {return read_fd;}
int child::get_write_fd() {return write_fd;}
unsigned int child::get_pr_number() {return pr_number;}
std::string child::get_dirs() {return dirs;}
std::string** child::get_dirs_ptr() {return dirs_ptr;}

// Setters
void child::set_pid(pid_t PID) {pid=PID;}
void child::set_read_fd(int READ_FD) {read_fd=READ_FD;}
void child::set_write_fd(int WRITE_FD) {write_fd=WRITE_FD;}
void child::set_pr_number(unsigned int PR_NUMBER){pr_number=PR_NUMBER;}
void child::set_dirs(std::string DIRS) {dirs=DIRS;}
void child::set_dirs_ptr(std::string** DIRS_PTR) {dirs_ptr=DIRS_PTR;}