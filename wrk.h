#ifndef WRK_H
#define WRK_H

// Οι προαπαιτουμενες βιβλιοθηκες για τους workers
#include "common.h"

// Διαφορες δηλωσεις συναρτησεων
void worker_sig_handler(int);

// Τα arguments: PATH, fifoname, process_number, buffer_size, ...

class wrk : public proc_mask, public filedes_set
{
    private:

    // Το struct sigaction του εργατη
    struct sigaction worker_sig_set;

    // Οι βασικες μεταβλητες του εργατη
    int write_fd;
    int read_fd;
    unsigned int buffer_size;
    unsigned int blfl_size;
    DIR* dir_ptr;
    std::string** cnts_ptr;
    std::string input_dir_path;
    unsigned int pr_number;

    unsigned int total_requests;
    unsigned int valid_requests;
    unsigned int rejected_requests;

    // Οι δομες του εργατη
    text_files* textfilesptr;
    countries* countriesptr;
    citizens* citizensptr;
    viruses* virusesptr;


    public:
    // Αρχικοποιει το struct sigaction του εργατη - καλειται απ' τον Constructor
    void init_sigaction(void);

    // Constructor
    wrk(int, char**);

    // Getters
    int get_write_fd(void);
    int get_read_fd(void);
    unsigned int get_buffer_size(void);
    unsigned int get_blfl_size(void);
    DIR* get_dir_ptr(void);
    std::string** get_cnts_ptr(void);
    std::string get_input_dir_path(void);
    unsigned int get_pr_number(void);
    text_files* get_textfilesptr(void);
    countries* get_countriesptr(void);
    citizens* get_citizensptr(void);
    viruses* get_virusesptr(void);



    // DIRECTORIES - TEXT FILES - RECORDS - LOGFILE:
    void read_dir(void);
    //
    void read_text_file(std::string, std::string);
    // Ελεγχει αν μια εγγραφη υπαρχει στο database πριν την κανει insert. Αν ναι, τυπωνει σφαλμα. Αν οχι, την κανει insert
    void test_record(record_info);
    // Κανει insert μια εγγραφη στο database
    void insert_record(int,std::string,std::string,std::string,int,std::string,std::string,std::string);
    // Ελεγχει τα subdirectories για καινουρια text files - καλειται οταν ληφθει SIGUSR1
    void test_subdirectories(void);
    // Φτιαχνει το logfile
    void make_logfile(void);

    // Συναρτησεις ανταλλαγης μηνυματων
    int send_message(std::string);
    int send_message(const char*,unsigned int);
    std::string receive_message(void);
    message read_message(void);

    void read_from_fifo(void);
    void analyze_message(message);

    // SIGNALS:
    void serve_signals(void);

    // BLOOMFILTER:
    void send_bloomfilter(virus*);
    void send_bloomfilters(void);

    // Αλλες συναρτησεις
    void print_stats(void);
    void print_viruses(void);
};


#endif
