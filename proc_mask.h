#ifndef PROC_MASK_H
#define PROC_MASK_H

#include <iostream>
#include <signal.h>

class proc_mask
{
    protected:
    // 64 bits - 1 bit για καθε σημα
    char pending_signals[8];
    // Η μασκα του process
    sigset_t mask;

    public:
    // Constructor
    proc_mask(void);

    // Getters
    sigset_t get_mask(void);

    // Συναρτησεις διαχειρισης pending σηματων
    void clear_pend_sigs(void);
    int set_pend_sig(int);
    int clr_pend_sig(int);
    int sig_pending(int);
    void print_pend_sigs(void);

    // Συναρτησεις block - unblock - set
    sigset_t block_all(void);
    sigset_t set_mask(sigset_t);
    sigset_t unblock_all(void);

};

#endif