#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H

// Οι προαπαιτουμενες βιβλιοθηκες
#include "child.h"


// Χρησιμοποιειται απ' το class mst
struct process_info
{
    // Αν ειναι παιδι nullptr, αν ειναι ο πατερας εξαρταται
    child* process_pointer;
    // successful forks αν ειναι ο πατερας, ενας ακεραιος στο [1,+inf) αν ειναι παιδι
    unsigned int process_number;
    // 0 αν δεν συνεβη σφαλμα, ενας ακεραιος στο [1,+inf) αν συνεβη
    unsigned int error_number;
    // Η μασκα του process *πριν* την κληση της συναρτησης create_workers()
    sigset_t old_mask;
};

// Επιστρεφεται οταν ο πατερας ή το παιδι διαβαζουν ενα μηνυμα απ' το FIFO τυπου const char* , αντι για string
struct message
{
    char* message_ptr;
    unsigned int message_size;
};


#endif
