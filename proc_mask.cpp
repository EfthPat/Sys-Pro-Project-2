#include <cstring>
#include "proc_mask.h"

// Constructor
proc_mask::proc_mask()
{
    block_all();
    clear_pend_sigs();
}

// Getters
sigset_t proc_mask::get_mask() {return mask;}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Συναρτησεις διαχειρισης pending σηματων
void proc_mask::clear_pend_sigs()
{
    for(unsigned int i=0;i<8;i++)
        pending_signals[i]=0;
}

int proc_mask::set_pend_sig(int signal)
{
    if(signal<1 || signal>64)
        return -1;

    int signum=signal-1;
    int index=signum/8;
    int shift_amt=signum%8;
    pending_signals[index] |= 1<<(7-shift_amt);
    return 0;
}

int proc_mask::clr_pend_sig(int signal)
{
    if(signal<1 || signal>64)
        return -1;
    int signum=signal-1;
    int index=signum/8;
    int shift_amt=signum%8;
    char temp[1]={0};

    for(unsigned int i=0;i<8;i++)
        if(i!=shift_amt)
            temp[0]|=((1<<(7-i))&pending_signals[index]);
    pending_signals[index]=temp[0];
    return 0;
}

int proc_mask::sig_pending(int signal)
{
    if(signal<1 || signal>64)
        return -1;

    int signum=signal-1;
    int index=signum/8;
    int shift_amt=signum%8;
    char val=pending_signals[index]<<shift_amt;
    if(val<0)
        return 1;
    return 0;
}

void proc_mask::print_pend_sigs()
{
    std::cout<<"Pending Signals:"<<std::endl;
    for(int i=1;i<=64;i++)
        if(sig_pending(i)==1)
            std::cout<<i<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

sigset_t proc_mask::block_all()
{
    // Αποθηκευσε την παλια μασκα
    sigset_t old_mask=mask;
    // Φτιαξε ενα συνολο σηματων και γεμισε το
    sigset_t fullset;
    sigfillset(&fullset);
    // Βαλε το process να μπλοκαρει ολα τα σηματα
    sigprocmask(SIG_SETMASK, &fullset, nullptr);
    // Θεσε το γεματο συνολο ως τη νεα μασκα
    mask=fullset;
    // Γυρνα την παλια μασκα
    return old_mask;
}

sigset_t proc_mask::set_mask(sigset_t to_be_blocked)
{
    // Αποθηκευσε την παλια μασκα
    sigset_t old_mask=mask;
    // Βαλε το process να μπλοκαρει τα σηματα του συνολου to_be_blocked
    sigprocmask(SIG_SETMASK,&to_be_blocked, nullptr);
    // Θεσε το συνολο to_be_blocked ως τη νεα μασκα
    mask=to_be_blocked;
    // Γυρνα την παλια μασκα
    return old_mask;
}

sigset_t proc_mask::unblock_all()
{
    // Αποθηκευσε την παλια μασκα
    sigset_t old_mask=mask;
    // Φτιαξε ενα συνολο σηματων και αδειασε το
    sigset_t emptyset;
    sigemptyset(&emptyset);
    // Βαλε το process να ξεμπλοκαρει ολα τα σηματα
    sigprocmask(SIG_SETMASK,&emptyset, nullptr);
    // Θεσε το αδειο συνολο ως τη νεα μασκα
    mask=emptyset;
    // Γυρνα την παλια μασκα
    return old_mask;
}





