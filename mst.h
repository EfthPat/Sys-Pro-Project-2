#ifndef MST_H
#define MST_H

// Οι προαπαιτουμενες βιβλιοθηκες
#include "common.h"


// Διαφορες δηλωσεις συναρτησεων
void master_sig_handler(int);

// Το path της execl() για τα παιδια-processes και το ονομα του εκτελεσιμου των workers: PLACEHOLDERS
#define PATH "./Monitor"
#define WRK_EXEC "./Monitor"

// το ονομα του FIFO
#define fifoname "fifo"

// Το κατω και ανω φραγμα των workers
#define WRK_LL 1
#define WRK_UL 510


// Ο πατερας - master
class mst : public proc_mask, public filedes_set
{
    private:

    // Το struct sigaction του πατερα
    struct sigaction master_sig_set;

    // Οι βασικες μεταβλητες
    unsigned int buffer_size;
    unsigned int blfl_size;
    DIR* dir_ptr;
    std::string fifo_name;
    std::string input_dir_path;
    unsigned int max_pr_number;
    unsigned int mat_size;
    child* workersptr;

    unsigned int total_requests;
    unsigned int valid_requests;
    unsigned int rejected_requests;

    // Οι δομες του πατερα
    viruses* virusesptr;

    public:

    // Αρχικοποιει το struct sigaction του πατερα - καλειται απ' τον Constructor
    void init_sigaction(void);
    // Constructor -- buffer size, fifo name, ..
    mst(unsigned int,unsigned int,DIR*,std::string,std::string);

    // Getters
    unsigned int get_buffer_size(void);
    unsigned int get_blfl_size(void);
    DIR* get_dir_ptr(void);
    std::string get_fifo_name(void);
    unsigned int get_max_pr_number(void);
    unsigned int get_mat_size(void);
    const child* get_workersptr(void);

    // Setters
    void set_fifo_name(std::string);


    // WORKERS:
    // Δημιουργει (n) εργατες και (2*n) FIFOs
    process_info create_workers(unsigned int, unsigned int);
    // Ελεγχει αν οι εργατες ειναι ζωντανοι - καλειται απ' τον handler σε περιπτωση που σταλθει σημα SIGCHLD
    process_info check_workers(void);
    // Αναστενει εναν εργατη σε περιπτωση που πεθανει - καλειται μεσω της check_workers()
    process_info revive_worker(unsigned int);
    // Την καλει ενας εργατης, αφου δημιουργηθει, μεσω της create_workers() ή της revive_worker()
    void call_exec(process_info);


    // DIRECTORIES - LOGFILE :
    // Κατανεμει τα directories με round-robin κατα το τελος της create_workers()
    void distr_dirs(unsigned int);
    // Ελεγχει το τρεχον directory για αρχεια της μορφης U<PID>_
    void check_filesystem(void);
    void make_logfile(void);

    // SELECT():
    // Προσθετει ολους τους read και write file descriptors στα sets. Καλειται στο τελος της create_workers()
    process_info add_fds_to_set(void);
    // Ελεγχει αν οι read file descriptors ειναι ετοιμοι. Αν ναι, διαβαζει απο αυτους.
    void track_readfds(void);


    // Συναρτησεις ανταλλαγης μηνυματων
    process_info send_message(std::string, int);
    process_info send_message(const char*, unsigned int, int);
    int message_all(std::string);
    std::string receive_message(int);
    message read_message(int);


    // BLOOMFILTERS:
    void read_bloomfilters(void);
    void breakdown_bloomfilters(int);
    void update_bloomfilter(char*, char*);

    // SIGNAL HANDLING:
    void serve_signals(void);
    process_info kill_all(int);


    // QUERIES:

    int travelRequest(int,std::string,std::string,std::string,std::string);
    int travelStats_noarg(std::string,std::string,std::string);
    int travelStats_arg(std::string,std::string,std::string,std::string);
    int searchVaccinationStatus(int);
    int addVaccinationRecords(std::string);
    void ext(void);


    // STDIN:
    void read_command(void);

    // Σβηνει ολα τα FILES της μορφης FIFO*
    int delete_files(void);
    void free_mem(void);
    // Destructor - καλει την delete files
    ~mst();


    // Αλλες συναρτησεις
    bool error_check(unsigned int,unsigned int);
    void print_workers(void);
    void sort_dirs(std::string*,int,int);
    int find_pvt(std::string*,int,int);
};



#endif
