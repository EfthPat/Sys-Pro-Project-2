#include "mst.h"

using namespace std;

mst* global_ptr;
bool SIGINT_SIGQUIT_pending=false;

// Constructor
mst::mst(unsigned int BUFFER_SIZE, unsigned int BLFL_SIZE, DIR* DIR_PTR, std::string FIFO_NAME, std::string INPUT_DIR_PATH)
{
    global_ptr=this;

    // Αρχικοποιησε το struct sigaction του πατερα
    init_sigaction();

    // Αρχικοποιησε τις απλες μεταβλητες
    buffer_size=BUFFER_SIZE;
    blfl_size=BLFL_SIZE;
    dir_ptr=DIR_PTR;
    fifo_name=FIFO_NAME;
    input_dir_path=INPUT_DIR_PATH;
    max_pr_number=0;
    mat_size=0;
    workersptr=nullptr;

    total_requests=0;
    valid_requests=0;
    rejected_requests=0;

    // Οι δομες του πατερα:
    virusesptr=new viruses;
}

// Αρχικοποιει το struct sigaction του πατερα - καλειται απ' τον Constructor
void mst::init_sigaction()
{
    memset(&master_sig_set,0,sizeof(master_sig_set));
    // Αρχικοποιησε τα flags
    master_sig_set.sa_flags=SA_RESTART;
    // Βαλε ως signal handler την συναρτηση master_sig_handler
    master_sig_set.sa_handler=master_sig_handler;
    // Βαλε για ποια σηματα θα καλειται ο signal handler
    sigaction(SIGINT,&master_sig_set, nullptr);
    sigaction(SIGQUIT,&master_sig_set, nullptr);
    sigaction(SIGCHLD,&master_sig_set, nullptr);
    sigaction(SIGUSR1,&master_sig_set, nullptr);
    // Βαλε ολα τα σηματα να μπλοκαρονται/γινονται queue κατα την κληση του signal handler
    sigfillset(&master_sig_set.sa_mask);

}

// Getters
unsigned int mst::get_buffer_size() {return buffer_size;}
unsigned int mst::get_blfl_size() {return blfl_size;}
DIR* mst::get_dir_ptr() {return dir_ptr;}
std::string mst::get_fifo_name() {return fifo_name;}
unsigned int mst::get_max_pr_number() {return max_pr_number;}
unsigned int mst::get_mat_size() {return mat_size;}
const child *mst::get_workersptr() {return workersptr;}

// Setters
void mst::set_fifo_name(std::string FIFO_NAME) {fifo_name=FIFO_NAME;}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Κατανεμει τα directories στους εργατες
void mst::distr_dirs(unsigned int countries_found)
{
    DIR* temp_dir_ptr=opendir(input_dir_path.c_str());
    std::string* countries=new string[countries_found];
    unsigned int j=0, i=0;
    dirent* entry;
    while((entry=readdir(temp_dir_ptr))!=nullptr)
    {
        if(entry->d_name[0]!='.')
        {
            countries[j].append(entry->d_name);
            j++;
        }
    }
    closedir(temp_dir_ptr);

    sort_dirs(countries,0,countries_found-1);

    std::string* names=new std::string[mat_size];

    for(i=0;i<countries_found;i++)
    {
        j=i%mat_size;
        names[j].append(countries[i]);
        names[j].push_back(' ');
    }

    // Για καθε εργατη
    for(i=0;i<mat_size;i++)
    {
        // Αποθηκευσε τα directories του στον πινακα εργατων
        workersptr[i].set_dirs(names[i]);
        // Μετατρεψε τα σε tokens και αποθηκευσε τον δεικτη τυπου string**
        workersptr[i].set_dirs_ptr(comm_handler::tokenize(names[i].c_str()));
        // Γραψε στο FIFO του εργατη τα directories
        send_message(names[i],workersptr[i].get_write_fd());
    }

    delete[] countries;
    delete[] names;
}
// Ελεγχει το τρεχον directory για αρχεια της μορφης U<PID>_ . Καλειται οταν ληφθει σημα SIGUSR1
void mst::check_filesystem()
{
    // Ανοιξε το τρεχον directory
    DIR* curr_dir=opendir("./");
    DIR* temp_curr_cir=curr_dir;
    LOOK_FOR_UPDT:
    // Παρε το τρεχον αρχειο
    dirent* current_entry=readdir(curr_dir);
    if(current_entry!=nullptr)
    {
        // Αν το ονομα του αρχειου ξεκιναει απο 'U'
        if(current_entry->d_name[0]=='U')
        {
            std::string str_pid;
            for(int i=1;current_entry->d_name[i]!='_';i++)
                str_pid.push_back(current_entry->d_name[i]);
            // Βρες το pid του παιδιου
            int pid=std::stoi(str_pid);
            // Βρες σε ποιον read file descriptor αντιστοιχει το συγκεκριμενο pid
            for(int i=0;i<mat_size;i++)
            {
                if(workersptr[i].get_pid()==pid)
                {
                    int read_fd=workersptr[i].get_read_fd();
                    breakdown_bloomfilters(read_fd);
                }

            }
            // Σβησε το αρχειο
            std::string u_filename(current_entry->d_name);
            unlink(u_filename.c_str());
        }
        goto LOOK_FOR_UPDT;
    }
    // Κλεισε το directory
    closedir(temp_curr_cir);
}
// Φτιαχνει το logfile. Καλειται οταν ο πατερας λαβει σημα  SIGINT ή SIGQUIT
void mst::make_logfile()
{
    std::string pathname="./log_file.";
    std::string str_pid=std::to_string(getpid());
    pathname.append(str_pid);
    ofstream logfile(pathname);

    for(unsigned int i=0;i<mat_size;i++)
    {
        std::string** str_ptr=workersptr[i].get_dirs_ptr();
        for(unsigned int j=0;str_ptr[j]!=nullptr;j++)
            logfile<<str_ptr[j][0]<<std::endl;
    }
    logfile<<"TOTAL TRAVEL REQUESTS "<<total_requests<<std::endl;
    logfile<<"ACCEPTED "<<valid_requests<<std::endl;
    logfile<<"REJECTED "<<rejected_requests<<std::endl;
    logfile.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Δημιουργει (n) workers και (2*n) FIFOs, 2 FIFOs για καθε worker.
// ERRORS: 1 -> Workers not in domain, 2 -> mkfifo() failed, 4 -> fork() failed, 8 -> open() failed
process_info mst::create_workers(unsigned int workers, unsigned int countries_found)
{
    sigset_t old_mask=block_all();

    std::string full_name_p="fifo_P_";
    std::string full_name_c="fifo_C_";

    // Φτιαξε ενα πινακα μεγεθους <workers> για την αποθηκευση των process ids / file descriptors / pr_numbers
    workersptr=new child[workers];
    mat_size=workers;

    // Για καθε worker
    for(unsigned int i=0; i<workers; i++)
    {
        // Υπολογισε τον καταληκτικο αριθμο και προσθεσε τον στο τελος του ονοματος του FIFO
        std::string number=std::to_string(max_pr_number);
        std::string p_pipe=full_name_p+number;
        std::string c_pipe=full_name_c+number;

        workersptr[i].set_pr_number(max_pr_number);

        // Φτιαξε τα 2 FIFOs
        int mkfifo_p_val=mkfifo(p_pipe.c_str(), FIFO_P_PERMS);
        int mkfifo_c_val=mkfifo(c_pipe.c_str(), FIFO_C_PERMS);

        // Κανε fork
        pid_t fork_id=fork();
        // Αν εισαι παιδι, κανε delete[] τον πινακα που κληρονομησες και καλεσε την execl()
        if(fork_id==0)
        {
            delete[] workersptr;
            call_exec({nullptr, max_pr_number, 0, old_mask});
        }

        workersptr[i].set_pid(fork_id);

        // Ανοιξε το FIFO_C για READ σε NONBLOCK mode
        int read_fd=open(c_pipe.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        // Ανοιξε το FIFO_P για WRITE με FLAG=RDWR
        int write_fd=open(p_pipe.c_str(), O_RDWR | O_CLOEXEC);

        // Αποθηκευσε τους READ και WRITE file descriptors
        workersptr[i].set_read_fd(read_fd);
        workersptr[i].set_write_fd(write_fd);

        fd_set tmp_set;
        FD_ZERO(&tmp_set);
        do
        {
            FD_SET(write_fd,&tmp_set);
            select(max_fd, nullptr,&tmp_set, nullptr,&timeout);
        }while(FD_ISSET(write_fd,&tmp_set)==0);

        // Στειλε στο παιδι τα BUFFER_SIZE, BLOOMFILTER_SIZE, INPUT_DIR_PATH
        std::string tmp_msg=std::to_string(buffer_size)+" "+std::to_string(blfl_size)+" "+input_dir_path;
        send_message(tmp_msg,write_fd);

        max_pr_number++;
    }

    // Αφου τελειωσεις με τα forks και τα FIFOs, βαλε ολους τους file descriptors στα συνολα read_fds και write_fds και κανε distribute τα directories
    add_fds_to_set();
    distr_dirs(countries_found);

    return {workersptr, 0, 0, old_mask};
}

// Ελεγχει αν οι εργατες ειναι ζωντανοι. Καλειται μεσω του handler ή της serve_signals() αν ληφθει σημα SIGCHLD
// ERRORS: 1 -> open_for_read failed, 2 -> open_for_write failed, 4 -> close() failed, ..
process_info mst::check_workers()
{
    // Μπλοκαρε ολα τα σηματα και αποθηκευσε την παλια μασκα για να τη γυρισεις μολις τελειωσει η συναρτηση
    sigset_t old_mask=block_all();

    // Ο αριθμος των επιτυχων forks
    unsigned int succ_forks=0;
    // Ο αριθμος και τα flags σφαλματων
    unsigned int error_sum=0;
    bool ofr_failed=false, ofw_failed=false, close_failed=false;
    int status;
    process_info packet;

    unsigned int i=0;
    // Για καθε εργατη
    while(i<mat_size)
    {
        // Ελεγξε αν ο εργατης εχει πεθανει
        pid_t res=waitpid(workersptr[i].get_pid(),&status,WNOHANG);
        // Αν ο εργατης πεθανε
        if(res==workersptr[i].get_pid())
        {
            // Αναστησε τον εργατη
            packet=revive_worker(i);
            succ_forks+=packet.process_number;
            // Αν ενα τουλαχιστον open() για read η write, η ενα close() αποτυχει, σημειωσε το στο error_sum
            if(error_check(packet.error_number,1) && !ofr_failed)
            {
                ofr_failed=true;
                error_sum+=1;
            }
            if(error_check(packet.error_number,2) && !ofw_failed)
            {
                ofw_failed=true;
                error_sum+=2;
            }
            if(error_check(packet.error_number,4) && !close_failed)
            {
                close_failed=true;
                error_sum+=4;
            }
        }
        i++;
    }
    return {workersptr,succ_forks,error_sum,old_mask};
}

// Καλειται αν ενας εργατης πεθανει. Στα mkfifo() και fork() επιμενει μεχρι επιτυχιας
// ERRORS: 1 -> open_for_read failed, 2 -> open_for_write failed, 4 -> close() failed, ..
process_info mst::revive_worker(unsigned int worker_index)
{

    // Μπλοκαρε ολα τα σηματα και αποθηκευσε την παλια μασκα για να τη γυρισεις μολις τελειωσει η συναρτηση
    sigset_t old_mask=block_all();

    // Ο αριθμος σφαλματων που μπορει να συμβουν
    unsigned int error_sum=0;

    int close_res1=0, close_res2=0;
    // Παρε τα ακρα του FIFO του σκοτωμενου εργατη
    int killed_read_fd=workersptr[worker_index].get_read_fd();
    int killed_write_fd=workersptr[worker_index].get_write_fd();
    // Αν ο read file descriptor ειναι εγκυρος, βγαλ' τον απ' το read set και κλεισ' τον
    if(killed_read_fd>=0)
    {
        FD_CLR(killed_read_fd,&read_fds);
        close_res1=close(killed_read_fd);
    }
    // Αν ο write file descriptor ειναι εγκυρος, βγαλ' τον απ' το write set και κλεισ' τον
    if(killed_write_fd>=0)
    {
        FD_CLR(killed_write_fd,&write_fds);
        close_res2=close(killed_write_fd);
    }
    // Αν τουλαχιστον ενα close() αποτυχει, σημειωσε το στο error_sum
    if(close_res1==-1 || close_res2==-1)
        error_sum+=4;

    // Σβησε τα FIFOs του σκοτωμενου εργατη, αφου τα κλεισεις πρωτα
    std::string killed_p_pipe=fifo_name+FIFO_P_SUFFIX+std::to_string(workersptr[worker_index].get_pr_number());
    std::string killed_c_pipe=fifo_name+FIFO_C_SUFFIX+std::to_string(workersptr[worker_index].get_pr_number());
    unlink(killed_p_pipe.c_str());
    unlink(killed_c_pipe.c_str());

    // Υπολογισε τα ονοματα των νεων FIFOs
    std::string number=std::to_string(max_pr_number);
    std::string p_pipe=fifo_name+FIFO_P_SUFFIX+number;
    std::string c_pipe=fifo_name+FIFO_C_SUFFIX+number;

    // Φτιαξε το P_FIFO - επιμενει
    int mkfifo_p_val;
    do
        mkfifo_p_val=mkfifo(p_pipe.c_str(), FIFO_P_PERMS);
    while(mkfifo_p_val==-1);

    // Φτιαξε το C_FIFO - επιμενει
    int mkfifo_c_val;
    do
        mkfifo_c_val=mkfifo(c_pipe.c_str(), FIFO_C_PERMS);
    while(mkfifo_c_val==-1);

    // Κανε το fork() - επιμενει
    pid_t fork_id;
    do
        fork_id=fork();
    while(fork_id==-1);

    // Αν εισαι το παιδι, καλεσε την execl()
    if(fork_id==0)
        call_exec({nullptr,max_pr_number,0,old_mask});

    // Αν εισαι ο πατερας
    // Ανοιξε το FIFO_C για READ σε NONBLOCK mode
    int read_fd=open(c_pipe.c_str(),O_RDONLY|O_NONBLOCK|O_CLOEXEC);
    // Αν αποτυχει, σημειωσε το στο error_sum
    if(read_fd==-1)
        error_sum+=1;
    // Αλλιως, βαλε τον read file descriptor στο read set της select()
    else
        add_fd_rd(read_fd);

    // Ανοιξε το FIFO_P για WRITE με FLAG=RDWR
    int write_fd=open(p_pipe.c_str(),O_RDWR|O_CLOEXEC);
    // Αν αποτυχει, σημειωσε το στο error_sum
    if(write_fd==-1)
        error_sum+=2;
    // Αλλιως, βαλε τον write file descriptor στο write set της select()
    else
        add_fd_wr(write_fd);


    // Αποθηκευσε στο κουτακι τα στοιχεια του νεου process και αυξησε το μεγιστο process number
    workersptr[worker_index].set_pr_number(max_pr_number);
    workersptr[worker_index].set_pid(fork_id);
    workersptr[worker_index].set_read_fd(read_fd);
    workersptr[worker_index].set_write_fd(write_fd);
    max_pr_number++;

    fd_set tmp;
    FD_ZERO(&tmp);
    do
    {
        FD_SET(write_fd,&tmp);
        select(max_fd, nullptr, &tmp, nullptr, &timeout);
    }while(FD_ISSET(write_fd,&tmp)==0);

    // Στειλε στο παιδι τα BUFFER_SIZE, BLOOMFILTER_SIZE, INPUT_DIR_PATH
    std::string tmp_msg=std::to_string(buffer_size)+" "+std::to_string(blfl_size)+" "+input_dir_path;
    send_message(tmp_msg,write_fd);

    // Στειλε στο FIFO του παιδιου τις χωρες που πρεπει να διαβασει
    send_message(workersptr[worker_index].get_dirs(), write_fd);

    FD_ZERO(&tmp);
    do
    {
        FD_SET(read_fd,&tmp);
        select(max_fd,&tmp, nullptr, nullptr, &timeout);
    }while(FD_ISSET(read_fd,&tmp)==0);

    // Διαβασε τα ανανεωμενα bloom filters που σου εστειλε
    breakdown_bloomfilters(read_fd);

    return {workersptr,1,error_sum,old_mask};
}

// Καλειται μονο απο εναν εργατη μεσω της create_workers() η της revive_worker()
void mst::call_exec(process_info packet)
{
    // Αν εισαι παιδι
    if(packet.process_pointer==nullptr && packet.error_number==0)
    {
        // Στειλε στη main() του worker τα: Path, Monitor Exec, και τον καταληκτικο αριθμο FIFO για να ξερεις απο ποιο FIFO να διαβαζεις/γραφεις
        std::string code=std::to_string(packet.process_number);
        execl(PATH,WRK_EXEC,code.c_str(),nullptr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Προσθετει τους read και write file edscriptors στα sets της select(). Καλειται στο τελος της create_workers()
// ERRORS: 1 -> invalid read file descriptor, 2 -> invalid write file descriptor, ..
process_info mst::add_fds_to_set()
{
    // Μπλοκαρε ολα τα σηματα και αποθηκευσε την παλια μασκα για να τη γυρισεις μολις τελειωσει η συναρτηση
    sigset_t old_mask=block_all();

    // Ο αριθμος και τα flags σφαλματων
    unsigned int error_sum=0;
    bool rd_fd_fail=false, wr_fd_fail=false;

    unsigned int i=0;
    int read_filedes, write_filedes, res;


    // Για καθε εργατη
    while(i<mat_size)
    {
        // Παρε τους δυο read και write file descriptors
        read_filedes=workersptr[i].get_read_fd();
        write_filedes=workersptr[i].get_write_fd();
        // Επιχειρησε να βαλεις τον read file descriptor στο read set
        res=add_fd_rd(read_filedes);
        // Αν αποτυχει και ειναι η πρωτη φορα για τους read fds, σημειωσε το στο error_sum
        if(res==-1 && !rd_fd_fail)
        {
            rd_fd_fail=true;
            error_sum+=1;
        }
        // Επιχειρησε να βαλεις τον write file descriptor στο write set
        res=add_fd_wr(write_filedes);
        // Αν αποτυχει και ειναι η πρωτη φορα για τους write fds, σημειωσε το στο error_sum
        if (res==-1 && !wr_fd_fail)
        {
            wr_fd_fail=true;
            error_sum+=2;
        }
        i++;
    }

    return {workersptr,0,error_sum,old_mask};
}


// Ελεγχει ποιοι read file descriptors ειναι ετοιμοι για διαβασμα. Αν καποιος ειναι ετοιμος, διαβαζει απο αυτον
void mst::track_readfds()
{
    fd_set temp_read_fds=read_fds;
    int ready_fds, current_read_fd;
    std::string received_message;
    unsigned int i=0;

    // Βρες ποσοι file descriptors ειναι ετοιμοι για διαβασμα
    ready_fds=select(max_fd,&temp_read_fds, nullptr, nullptr, &timeout);

    // Για καθε read file descriptor στον πινακα
    while(i<mat_size && ready_fds>0)
    {
        // Αν ο file descriptor ειναι ετοιμος
        current_read_fd=workersptr[i].get_read_fd();
        if(FD_ISSET(current_read_fd,&temp_read_fds)!=0)
        {
            // Actions
            received_message=receive_message(current_read_fd);
            ready_fds--;
        }
        i++;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ERRORS: 1 -> write() failed, 2 -> write_fd invalid, ..
process_info mst::send_message(std::string message, int write_fd)
{

    // Μπλοκαρε ολα τα σηματα και αποθηκευσε την παλια μασκα για να τη γυρισεις μολις τελειωσει η συναρτηση
    sigset_t old_mask=block_all();

    message.push_back('\0');

    // Υπολογισε το κωδικοποιημενο μηνυμα και το μηκος του
    unsigned int msg_size=message.size();
    std::string encoded=std::to_string(msg_size);
    encoded.push_back('_');
    unsigned int final_msg_size=encoded.size()+msg_size;
    encoded.append(message);

    // Ορισε τον buffer και αρχικοποιησε τον με το συμβολο BUFF_INIT
    char buffer[buffer_size];
    memset(buffer,BUFF_INIT,buffer_size);
    // Κανε την μετατροπη τυπου απο string σε const char*
    const char* str_ptr=encoded.c_str();

    unsigned int j=0, k;

    CP_TO_BUFF:
    k=0;
    while(j<final_msg_size && k<buffer_size)
    {
        buffer[k]=str_ptr[j];
        k++;
        j++;
    }
    if(k==buffer_size && j!=final_msg_size)
    {
        int bytes_written=write(write_fd,buffer,buffer_size);
        if(bytes_written==-1)
            return {workersptr,0,1,old_mask};
        goto CP_TO_BUFF;
    }
    else
    {
        int bytes_written=write(write_fd,buffer,k);
        if(bytes_written==-1)
            return {workersptr,0,1,old_mask};
    }
    return {workersptr,0,0,old_mask};
}

process_info mst::send_message(const char* message, unsigned int length, int write_fd)
{
    // Μπλοκαρε ολα τα σηματα και αποθηκευσε την παλια μασκα για να τη γυρισεις μολις τελειωσει η συναρτηση
    sigset_t old_mask=block_all();

    // Αν ο file descriptor δεν ανηκει στο write_set, γυρνα σφαλμα
    if(FD_ISSET(write_fd,&write_fds)==0)
        return{workersptr,0,2,old_mask};

    // Υπολογισε το κωδικοποιημενο μηνυμα και το μηκος του
    std::string encoded=std::to_string(length);
    encoded.push_back('_');
    unsigned int msg_size=encoded.size();

    // Ορισε τον buffer και αρχικοποιησε τον με το συμβολο BUFF_INIT
    char buffer[buffer_size];
    memset(buffer,BUFF_INIT,buffer_size);
    // Κανε την μετατροπη τυπου απο string σε const char* για το encoded
    const char* str_ptr=encoded.c_str();
    bool encoded_sent=false;

    unsigned int j=0, k;

    CP_TO_BUFF:
    k=0;
    while(j<msg_size && k<buffer_size)
    {
        if(!encoded_sent)
            buffer[k]=str_ptr[j];
        else
            buffer[k]=message[j];
        k++;
        j++;
    }
    if(k==buffer_size && j!=msg_size)
    {
        int bytes_written=write(write_fd,buffer,buffer_size);
        if(bytes_written==-1)
            return {workersptr,0,1,old_mask};
        goto CP_TO_BUFF;
    }

        int bytes_written=write(write_fd,buffer,k);
        if(bytes_written==-1)
            return {workersptr,0,1,old_mask};

    if(!encoded_sent)
    {
        encoded_sent=true;
        j=0;
        msg_size=length;
        goto CP_TO_BUFF;
    }

    return {workersptr,0,0,old_mask};
}

// Στελνει ενα μηνυμα σε ολα τα παιδια - workers
// ERRORS: 1 -> write_fd invalid, 2 -> write() failed, ..
int mst::message_all(std::string message)
{
    // Ο αριθμος και τα flags σφαλματων που μπορει να συμβουν
    unsigned int error_sum=0;
    bool write_fd_fail=false, write_failed=false;

    // Υπολογισε το κωδικοποιημενο μηνυμα και το μηκος του
    unsigned int msg_size=message.size();
    std::string encoded=std::to_string(msg_size);
    encoded.push_back('_');
    unsigned int final_msg_size=encoded.size()+msg_size;
    encoded.append(message);

    // Ορισε τον buffer και αρχικοποιησε τον με το συμβολο BUFF_INIT
    char buffer[buffer_size];
    memset(buffer,BUFF_INIT,buffer_size);
    // Κανε την μετατροπη τυπου απο string σε const char*
    const char* str_ptr=encoded.c_str();

    // Για καθε εργατη
    for(unsigned int i=0;i<mat_size;i++)
    {
        // Παρε τον write file descriptor
        int write_fd=workersptr[i].get_write_fd();
        // Αν ο file descriptor δεν ειναι εγκυρος και βρισκουμε πρωτη φορα μη εγκυρο fd, σημειωσε το στο error sum
        if(fd_in_wr(write_fd)!=1)
        {
            if(!write_fd_fail)
            {
                write_fd_fail=true;
                error_sum+=1;
            }
            continue;
        }
        // Ο file descriptor ειναι εγκυρος
        unsigned int j=0, k;

        CP_TO_BUFF:
        k=0;
        while(j<final_msg_size && k<buffer_size)
        {
            buffer[k]=str_ptr[j];
            k++;
            j++;
        }
        if(k==buffer_size && j!=final_msg_size)
        {
            int bytes_written=write(write_fd,buffer,buffer_size);
            if(bytes_written==-1)
            {
                if(!write_failed)
                {
                    write_failed=true;
                    error_sum+=2;
                }
                continue;
            }
            goto CP_TO_BUFF;
        }
        else
        {
            int bytes_written=write(write_fd,buffer,k);
            if(bytes_written==-1)
            {
                if(!write_failed)
                {
                    write_failed=true;
                    error_sum+=2;
                }
                continue;
            }
        }
    }
    return error_sum;
}

// ERRORS: "*R_F*" -> read() failed, "*R_F_I*" -> read_fd invalid
std::string mst::receive_message(int read_fd)
{
    sigset_t old_mask=block_all();

    if(FD_ISSET(read_fd,&read_fds)==0)
    {
        set_mask(old_mask);
        return "*R_F_I*";
    }

    // Δημιουργησε τον buffer και αρχικοποιησε τον με το συμβολο BUFF_INIT
    char buffer[buffer_size];
    memset(buffer,BUFF_INIT,buffer_size);

    char temp_buff[1];
    bool found_end=false;
    int bytes_read;
    std::string rd_amt;

    do
    {
        ATTEMPT_TO_READ:
            bytes_read=read(read_fd,temp_buff,1);
            if(bytes_read==-1)
            {
                if(errno==EAGAIN || errno==EWOULDBLOCK)
                    goto ATTEMPT_TO_READ;
                else
                {
                    set_mask(old_mask);
                    return "*R_F*";
                }
            }
            if(temp_buff[0]=='_')
                found_end=true;
            else
                rd_amt.push_back(temp_buff[0]);
    }while(!found_end);

    int read_amount=std::stoi(rd_amt);

    unsigned int min_amount;

    std::string message;
    while(read_amount>0)
    {
        if(buffer_size>read_amount)
            min_amount=read_amount;
        else
            min_amount=buffer_size;

        ATTEMPT_TO_READ_2:
        bytes_read=read(read_fd,buffer,min_amount);
        if(bytes_read==-1)
        {
            if(errno==EAGAIN || errno==EWOULDBLOCK)
                goto ATTEMPT_TO_READ_2;
            else
            {
                set_mask(old_mask);
                return "*R_F*";
            }
        }
        for(unsigned int i=0;i<bytes_read;i++)
            message.push_back(buffer[i]);
        read_amount-=bytes_read;
    }
    set_mask(old_mask);
    return message;
}

message mst::read_message(int read_fd)
{
    sigset_t old_mask=block_all();

    if(FD_ISSET(read_fd,&read_fds)==0)
    {
        set_mask(old_mask);
        return {nullptr,0};
    }

    // Δημιουργησε τον buffer και αρχικοποιησε τον με το συμβολο BUFF_INIT
    char buffer[buffer_size];
    memset(buffer,BUFF_INIT,buffer_size);

    char temp_buff[1];
    bool found_end=false;
    int bytes_read;
    std::string rd_amt="0";

    do
    {
        ATTEMPT_TO_READ:
        bytes_read=read(read_fd,temp_buff,1);
        if(bytes_read==-1)
        {
            if(errno==EAGAIN || errno==EWOULDBLOCK)
                goto ATTEMPT_TO_READ;
            else
            {
                set_mask(old_mask);
                return {nullptr,0};
            }
        }
        if(temp_buff[0]=='_')
            found_end=true;
        else
            rd_amt.push_back(temp_buff[0]);
    }while(!found_end);

    // Υπολογισε το μεγεθος που πρεπει να διαβασεις και φτιαξε ενα πινακα τετοιου μεγεθους
    unsigned int read_amount=std::stoi(rd_amt);
    char* message=new char[read_amount];
    int min_amount;
    unsigned int init_read_amount=read_amount;


    // Το j ειναι για την κινηση πανω στον πινακα message
    unsigned int j=0;
    while(read_amount>0)
    {
        if(buffer_size>read_amount)
            min_amount=read_amount;
        else
            min_amount=buffer_size;

        ATTEMPT_TO_READ_2:
        bytes_read=read(read_fd,buffer,min_amount);
        if(bytes_read==-1)
        {
            if(errno==EAGAIN || errno==EWOULDBLOCK)
                goto ATTEMPT_TO_READ_2;
            else
            {
                set_mask(old_mask);
                return {message,0};
            }
        }
        for(unsigned int i=0;i<bytes_read;i++)
        {
            message[j]=buffer[i];
            j++;
        }
        read_amount-=bytes_read;
    }

    set_mask(old_mask);
    return {message,init_read_amount};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Παιρνει τα bloomfilters για ΚΑΘΕ εργατη
void mst::read_bloomfilters()
{
    fd_set served;
    FD_ZERO(&served);

    // Βρες ποσοι εργατες υπαρχουν
    unsigned int workers=mat_size, i;
    fd_set temp_read_fds;
    int current_read_fd;

    CHECK_AVAILABILITY:

    // Γεμισε το συνολο των read file descriptors
    temp_read_fds=read_fds;
    // Βρες ποσοι read_fds ειναι ετοιμοι για διαβασμα
    select(max_fd,&temp_read_fds, nullptr, nullptr, &timeout);

    // Για καθε read file descriptor στον πινακα
    for(unsigned int i=0;i<mat_size;i++)
    {
        current_read_fd=workersptr[i].get_read_fd();
        // Αν ο file descriptor ειναι ετοιμος και δεν εχει εξυπηρετηθει πιο πριν
        if(FD_ISSET(current_read_fd,&temp_read_fds)!=0 && FD_ISSET(current_read_fd,&served)==0)
        {
            // Διαβασε ολα τα bloom filters που σου εστειλε ο εργατης στο συγκεκριμενο ακρο
            breakdown_bloomfilters(current_read_fd);
            FD_SET(current_read_fd,&served);
            workers--;
        }
    }
    // Αν υπαρχουν ακομα εργατες που δεν ειναι ετοιμοι, προσπαθησε να ξαναδιαβασεις
    if(workers>0)
        goto CHECK_AVAILABILITY;
}

void mst::breakdown_bloomfilters(int read_fd)
{
    READ_FROM_FIFO:
    unsigned int i=0, j=0;
    // Διαβασε απ' τον read file descriptor τα virus name + bloomfilter
    message msg=read_message(read_fd);
    char* msg_ptr=msg.message_ptr;
    unsigned int msg_size=msg.message_size;

    // Αν ο εργατης εστειλε το μηνυμα 'R', σβησε τον πινακα του μηνυματος και βγες απ' τη συναρτηση
    if(msg_size==1 && msg_ptr[0]=='R')
    {
        delete[] msg_ptr;
        return ;
    }

    // Υπολογισε το ονομα του ιου
    std::string virus_name;
    for(i=0;msg_ptr[i]!='_';i++)
        virus_name.push_back(msg_ptr[i]);
    i++;

    // Βαλε το bloom filter του εργατη σε ενα προσωρινο bloom filter
    char* blfl_ptr=new char[blfl_size];
    while(i<msg_size)
    {
        blfl_ptr[j]=msg_ptr[i];
        j++;
        i++;
    }

    // Βαλε τον ιο στην λιστα ιων και παρε τη διευθυνση του
    virus* virus_ptr=virusesptr->insert(virus_name,blfl_size);
    // Παρε το bloomfilter του ιου
    char* virus_blfl=virus_ptr->get_blfl()->get_ptr();
    // Καν' το update με αυτο που σου δωσε ο εργατης
    update_bloomfilter(virus_blfl,blfl_ptr);

    delete[] blfl_ptr;
    delete[] msg.message_ptr;
    goto READ_FROM_FIFO;
}

void mst::update_bloomfilter(char* old_bloomfilter, char* new_bloomfilter)
{
    for(unsigned int i=0;i<blfl_size;i++)
        old_bloomfilter[i]|=new_bloomfilter[i];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void master_sig_handler(int signum)
{
    global_ptr->set_pend_sig(signum);
}

void mst::serve_signals()
{
    block_all();
    // Αν το σημα SIGCHLD εκκρεμει
    if(sig_pending(SIGCHLD)==1)
    {
        // Ελεγξε αν οι εργατες ειναι ζωντανοι
        check_workers();
        // Αφαιρεσε το σημα SIGCHLD απ' τα pending signals
        clr_pend_sig(SIGCHLD);
    }

    // Αν το σημα SIGUSR1 εκκρεμει
    if(sig_pending(SIGUSR1)==1)
    {
        // Ελεγξε το τρεχον directory για αρχεια της μορφης U<PID>_
        check_filesystem();
        // Αφαιρεσε το σημα SIGUSR1 απ' τα pending signals
        clr_pend_sig(SIGUSR1);
    }

    // Αν τα σηματα SIGINT ή SIGQUIT εκκρεμουν
    if(sig_pending(SIGINT)==1 || sig_pending(SIGQUIT)==1)
    {
        if(!SIGINT_SIGQUIT_pending)
        {
            SIGINT_SIGQUIT_pending=true;
            clr_pend_sig(SIGINT);
            clr_pend_sig(SIGQUIT);
        }
        else
        {
            SIGINT_SIGQUIT_pending=false;
            for(unsigned int i=0;i<mat_size;i++)
            {
                kill(workersptr[i].get_pid(),SIGKILL);
                wait(NULL);
            }
            make_logfile();
            clr_pend_sig(SIGINT);
            clr_pend_sig(SIGQUIT);
            free_mem();
            exit(0);
        }
    }
}

// Στελνει ενα σημα σε ολα τα παιδια - workers
// ERRORS: 1 -> signal not valid, ..
process_info mst::kill_all(int signum)
{
    // Μπλοκαρε ολα τα σηματα και αποθηκευσε την παλια μασκα για να τη γυρισεις στο τελος της συναρτησης
    sigset_t old_mask=block_all();

    if(signum<1 || signum>64)
        return {nullptr,0,1,old_mask};

    // Για καθε εργατη
    for(unsigned int i=0;i<mat_size;i++)
    {
        // Παρε το process ID του εργατη
        pid_t worker_pid=workersptr[i].get_pid();
        // Στειλ' του το σημα signum
        kill(worker_pid,signum);
    }

    return {nullptr,0,0,old_mask};
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// QUERIES:


int mst::travelRequest(int id,std::string date,std::string countryFrom,std::string countryTo,std::string disease)
{
    // Ελεγξε αν υπαρχει ο ιος
    virus* virus_ptr=virusesptr->member(disease);
    // Αν ο ιος δεν υπαρχει, τυπωσε σφαλμα
    if(virus_ptr==nullptr)
    {
        std::cout<<"ERROR: Virus "<<disease<<" does not exist."<<std::endl;
        return -1;
    }
    bool countryFrom_exists=false, countryTo_exists=false;

    int monitor_write_fd, monitor_read_fd, monitor2_write_fd;
    // Για καθε εργατη
    for(unsigned int i=0; i<mat_size && (!countryFrom_exists || !countryTo_exists);i++)
    {
        // Παρε τις χωρες του
        std::string** dirs_ptr=workersptr[i].get_dirs_ptr();
        for(unsigned int j=0;dirs_ptr[j]!=nullptr;j++)
        {
            // Ελεγξε αν καποια απο αυτες ειναι η countryFrom η countryTo
            if(dirs_ptr[j][0]==countryFrom)
            {
                countryFrom_exists=true;
                monitor_write_fd=workersptr[i].get_write_fd();
                monitor_read_fd=workersptr[i].get_read_fd();
            }
            if(dirs_ptr[j][0]==countryTo)
            {
                countryTo_exists=true;
                monitor2_write_fd=workersptr[i].get_write_fd();
            }
        }
    }
    // Αν τουλαχιστον μια απ' τις 2 χωρες δεν υπαρχει, τυπωσε σφαλμα
    if(!countryFrom_exists)
    {
        std::cout<<"ERROR: Country "<<countryFrom<<" does not exist."<<std::endl;
        return -1;
    }
    if(!countryTo_exists)
    {
        std::cout<<"ERROR: Country "<<countryTo<<" does not exist."<<std::endl;
        return -1;
    }

    // Ελεγξε αν στο monitor process που διαχειριζεται το countryFrom υπαρχει ιος με αυτο το ονομα
    std::string message1="D "+disease;
    send_message(message1,monitor_write_fd);
    std::string message2=receive_message(monitor_read_fd);
    // Απ' το "ΝΟ"
    if(message2[0]=='N')
    {
        std::cout<<"ERROR: Virus "<<disease<<" does not exist for child process handling "<<countryFrom<<"."<<std::endl;
        return -1;
    }

    // Στειλε στο monitor που διαχειριζεται το countryFrom το citizenID για να δει αν υπαρχει
    message1="A "+std::to_string(id);
    send_message(message1,monitor_write_fd);
    message2=receive_message(monitor_read_fd);
    // Απ' το "ΝΟ"
    if(message2[0]=='N')
    {
        cout<<"ERROR: CitizenID "<<id<<" does not exist for child process handling "<<countryFrom<<"."<<std::endl;
        return -1;
    }
    // Παρε το bloomfilter του ιου
    bloomfilter* blfl_ptr=virus_ptr->get_blfl();
    // Υπολογισε τα hash values του id
    std::string str_id=std::to_string(id);
    unsigned long* hash_values=blfl_ptr->hash_values_of(str_id);
    // Αυξησε τα total requests
    total_requests++;
    // Αν ο πολιτης δεν ειναι εμβολιασμενος κατα του ιου, απορριψε το request
    if(blfl_ptr->val_exists(hash_values)==0)
    {
        std::cout<<"REQUEST REJECTED - YOU ARE NOT VACCINATED"<<std::endl;
        // Αποθηκευσε στα requests του ιου ενα νεο rejected request με την ημερομηνια και την χωρα countryTo
        virus_ptr->get_reqs()->insert(date,false,countryTo);
        rejected_requests++;
        send_message("E R",monitor2_write_fd);
        delete[] hash_values;
        return 0;
    }
    // Αλλιως, πακεταρε ολα τα δεδομενα για να τα στειλεις στο monitor process με κωδικο Β
    // B, ID, DATE, COUNTRYFROM, COUNTRYTO, VIRUS
    message1="B "+str_id+" "+date+" "+countryFrom+" "+countryTo+" "+disease;
    send_message(message1,monitor_write_fd);

    // Παρε την απαντηση απ' το monitor process
    message2=receive_message(monitor_read_fd);
    // Σπασε την απαντηση σε tokens
    std::string** str_ptr=comm_handler::tokenize(message2.c_str());
    // Αν δεν υπαρχει το id στη vaccinated skiplist του monitor, απορριψε το request
    if(str_ptr[0][0]=="NO")
    {
        std::cout<<"REQUEST REJECTED - YOU ARE NOT VACCINATED"<<std::endl;
        // Αποθηκευσε στα requests του ιου ενα νεο rejected request με την ημερομηνια και την χωρα
        virus_ptr->get_reqs()->insert(date,false,countryTo);
        rejected_requests++;
        comm_handler::delete_tokens(str_ptr);
        send_message("E R",monitor2_write_fd);
        delete[] hash_values;
        return 0;
    }
    // Αλλιως, ελεγξε αν το date εμβολιασμου ειναι το πολυ 6 μηνες νωριτερα απ' το date ταξιδιου
    std::string vacc_date=str_ptr[1][0];
    int date_result=comm_handler::date_gap(vacc_date,date);
    if(date_result==1)
    {
        std::cout<<"REQUEST ACCEPTED - HAPPY TRAVELS"<<std::endl;
        // Αποθηκευσε στα requests του ιου ενα νεο valid request με την ημερομηνια και την χωρα
        virus_ptr->get_reqs()->insert(date,true,countryTo);
        valid_requests++;
        send_message("E V",monitor2_write_fd);
    }
    else
    {
        std::cout<<"REQUEST REJECTED - YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE"<<std::endl;
        // Αποθηκευσε στα requests του ιου ενα νεο rejected request με την ημερομηνια και την χωρα
        virus_ptr->get_reqs()->insert(date, false, countryTo);
        rejected_requests++;
        send_message("E R",monitor2_write_fd);
    }
    comm_handler::delete_tokens(str_ptr);
    delete[] hash_values;
    return 0;
}

int mst::travelStats_noarg(std::string disease,std::string date1,std::string date2)
{
    unsigned int tmp_total_requests=0, tmp_valid_requests=0, tmp_rejected_requests=0;
    request_info temp;
    // Πηγαινε στον ιο
    virus* virusptr=virusesptr->member(disease);
    // Αν ο ιος δεν υπαρχει, τυπωσε σφαλμα
    if(virusptr==nullptr)
    {
        std::cout<<"ERROR: Virus "<<disease<<" does not exist."<<std::endl;
        return -1;
    }
    // Πηγαινε στους εργατες
    for(unsigned int i=0;i<mat_size;i++)
    {
        std::string** str_ptr=workersptr[i].get_dirs_ptr();
        // Για καθε χωρα που διαχειριζεται ο εργατης
        for(unsigned int j=0;str_ptr[j]!=nullptr;j++)
        {
            std::string curr_country=str_ptr[j][0];
            // Υπολογισε τα requests στην περιοδο [date1, date2] και προσθεσε τα στα ολικα requests ανα κατηγορια
            temp=virusptr->get_reqs()->calc_requests(date1,date2,curr_country);
            tmp_total_requests+=temp.total;
            tmp_valid_requests+=temp.valid;
            tmp_rejected_requests+=temp.rejected;
        }
    }
    std::cout<<"TOTAL REQUESTS "<<tmp_total_requests<<std::endl;
    std::cout<<"ACCEPTED "<<tmp_valid_requests<<std::endl;
    std::cout<<"REJECTED "<<tmp_rejected_requests<<std::endl;
    return 0;
}

int mst::travelStats_arg(std::string disease, std::string date1, std::string date2, std::string country)
{
    // Πηγαινε στους εργατες
    for(unsigned int i=0;i<mat_size;i++)
    {
        std::string** str_ptr=workersptr[i].get_dirs_ptr();
        for(unsigned int j=0;str_ptr[j]!=nullptr;j++)
            // Αν η χωρα υπαρχει, πηγαινε να ελεγξεις αν υπαρχει ο ιος
            if(str_ptr[j][0]==country)
                goto COUNTRY_EXISTS;
    }
    std::cout<<"ERROR: Country "<<country<<" does not exist."<<std::endl;
    return -1;
    COUNTRY_EXISTS:
    // Βρες τον ιο
    virus* virusptr=virusesptr->member(disease);
    // Αν ο ιος δεν υπαρχει, τυπωσε σφαλμα
    if(virusptr==nullptr)
    {
        std::cout<<"ERROR: Virus "<<disease<<" does not exist."<<std::endl;
        return -1;
    }
    requests* reqs=virusptr->get_reqs();
    // Υπολογισε ολα τα requests που εγιναν για την χωρα country στο πεδιο [date1, date2]
    request_info req_inf=reqs->calc_requests(date1,date2,country);
    // Τυπωσε τα total, accepted και rejected requests
    std::cout<<"TOTAL REQUESTS "<<req_inf.total<<std::endl;
    std::cout<<"ACCEPTED "<<req_inf.valid<<std::endl;
    std::cout<<"REJECTED "<<req_inf.rejected<<std::endl;
    return 0;
}

int mst::searchVaccinationStatus(int id)
{
    // Φτιαξε το μηνυμα
    std::string str_id=std::to_string(id);
    std::string request="C "+str_id;
    // Στειλε σε ολα τα child processes το request
    for(int i=0;i<mat_size;i++)
    {
        int write_fd=workersptr[i].get_write_fd();
        send_message(request,write_fd);
    }

    bool id_exists=false, positive_answer=false, duplicated_info=false;
    int current_read_fd, workers=mat_size;
    std::string current_answer, final_answer;
    fd_set served;
    FD_ZERO(&served);

    CHECK_AVAILABILITY:
    fd_set temp_read_fds=read_fds;
    select(max_fd,&temp_read_fds, nullptr, nullptr,&timeout);
    // Για καθε εργατη
    for(unsigned int i=0;i<mat_size;i++)
    {
        current_read_fd=workersptr[i].get_read_fd();
        // Αν ο file descriptor ειναι ετοιμος και δεν εχει εξυπηρετηθει πιο πριν
        if(FD_ISSET(current_read_fd,&temp_read_fds)!=0 && FD_ISSET(current_read_fd,&served)==0)
        {
            // Διαβασε την απαντηση που σου στειλε
            current_answer=receive_message(current_read_fd);
            // Αν η απαντηση ειναι θετικη
            if(current_answer[0]!='N')
            {
                // Σημειωσε πως το id υπαρχει σε καποιο child process
                id_exists=true;
                // Αν ειναι η πρωτη φορα
                if(!positive_answer)
                {
                    // Αποθηκευσε την απαντηση και σημειωσε οτι ελαβες θετικη απαντηση
                    final_answer=current_answer;
                    positive_answer=true;
                }
                // Αλλιως σημειωσε οτι υπαρχει σφαλμα διπλης εγγραφης
                else
                    duplicated_info=true;
            }
            FD_SET(current_read_fd,&served);
            workers--;
        }
    }
    if(workers>0)
        goto CHECK_AVAILABILITY;

    if(!id_exists)
    {
        std::cout<<"ERROR: citizenID "<<id<<" does not exist."<<std::endl;
        return -1;
    }
    if(duplicated_info)
    {
        std::cout<<"ERROR: citizenID "<<id<<" is duplicated among child processes."<<std::endl;
        return -1;
    }

    std::string** answer_ptr=comm_handler::tokenize(final_answer.c_str());
    std::cout<<id<<" "<<answer_ptr[1][0]<<" "<<answer_ptr[2][0]<<" "<<answer_ptr[3][0]<<std::endl;
    std::cout<<"AGE "<<answer_ptr[4][0]<<std::endl;
    for(unsigned int j=5;answer_ptr[j]!=nullptr;j++)
        comm_handler::an_str(answer_ptr[j][0]);

    comm_handler::delete_tokens(answer_ptr);
    return 0;
}

int mst::addVaccinationRecords(std::string country)
{
    bool country_found=false;
    fd_set temp_set;
    FD_ZERO(&temp_set);

    // Πηγαινε στον πινακα των εργατων
    for(unsigned int i=0;i<mat_size;i++)
    {
        std::string** str_ptr=workersptr[i].get_dirs_ptr();
        // Ψαξε τις χωρες που διαχειριζεται ο καθε εργατης
        for(unsigned int j=0;str_ptr[j]!=nullptr;j++)
        {
            // Αν η χωρα βρεθηκε
            if(str_ptr[j][0]==country)
            {
                // Παρε το pid του worker που διαχειριζεται την χωρα
                int worker_pid=workersptr[i].get_pid();
                // Στειλ' του το μηνυμα SIGUSR1
                kill(worker_pid,SIGUSR1);
                // Παρε τον read file descriptor του
                int read_fd=workersptr[i].get_read_fd();
                // Περιμενε μεχρι να γραψει το παιδι στο FIFO το καινουριο του bloom filter
                do
                {
                    FD_SET(read_fd,&temp_set);
                    select(max_fd,&temp_set, nullptr, nullptr,&timeout);
                }while(FD_ISSET(read_fd,&temp_set)==0);
                // Διαβασε το νεο bloom filter που σου εστειλε
                breakdown_bloomfilters(read_fd);
                // Σημειωσε οτι η χωρα βρεθηκε
                country_found=true;
                // Σβησε το αρχειο U<PID>_ που θα 'χει φτιαξει
                std::string u_name="./U"+std::to_string(worker_pid)+"_";
                unlink(u_name.c_str());
                // Σβησε το pending σημα SIGUSR1 που θα σου 'χει στειλει
                clr_pend_sig(SIGUSR1);
                // Βγες απ' το loop
                break;
            }
        }
        if(country_found)
            break;
    }
    if(!country_found)
    {
        std::cout<<"ERROR: Country "<<country<<" does not exist."<<std::endl;
        return -1;
    }
    return 0;
}

void mst::ext(void)
{
    for(unsigned int i=0;i<mat_size;i++)
    {
        kill(workersptr[i].get_pid(),SIGKILL);
        wait(NULL);
    }
    make_logfile();
    free_mem();
    exit(0);
}

// STDIN:

void mst::read_command(void)
{

    while(1)
    {
        std::string** tokens_ptr;
        std::string line;

        unblock_all();
        serve_signals();
        unblock_all();
        std::cout<<"InsertCommand>";
        // Διαβασε την γραμμη
        std::getline(std::cin,line);
        // Ξεμπλοκαρε τα σηματα και καν'τα serve πριν την εκτελεση της εντολης
        serve_signals();

        string_info str_info=comm_handler::tokenize_s(line.c_str());
        tokens_ptr=str_info.str_ptr;
        int tokens=str_info.tokens;

        if(tokens==0)
            std::cout<<"ERROR: Command not inserted."<<std::endl;
        // Query /travelRequest
        else if(tokens_ptr[0][0]=="/travelRequest")
        {
            if(tokens!=6)
                std::cout<<"ERROR: /travelRequest shall be called with exactly 5 arguments."<<std::endl;
            else if(comm_handler::val_id(tokens_ptr[1][0])==-1 || comm_handler::val_date(tokens_ptr[2][0])==-1 || comm_handler::val_flc(tokens_ptr[3][0])==-1)
                std::cout<<"ERROR: Some arguments are not in valid format."<<std::endl;
            else if(comm_handler::val_flc(tokens_ptr[4][0])==-1 || comm_handler::val_virus(tokens_ptr[5][0])==-1)
                std::cout<<"ERROR: Some arguments are not in valid format."<<std::endl;
            else
            {
                int id=std::stoi(tokens_ptr[1][0]);
                travelRequest(id,tokens_ptr[2][0],tokens_ptr[3][0],tokens_ptr[4][0],tokens_ptr[5][0]);
            }
        }
        // Query /searchVaccinationStatus
        else if(tokens_ptr[0][0]=="/searchVaccinationStatus")
        {
            if(tokens!=2)
                std::cout<<"ERROR: /searchVaccinationStatus shall be called with exactly 1 argument."<<std::endl;
            else if(comm_handler::val_id(tokens_ptr[1][0])==-1)
                std::cout<<"ERROR: Some arguments are not in valid format."<<std::endl;
            else
            {
                int id=std::stoi(tokens_ptr[1][0]);
                searchVaccinationStatus(id);
            }
        }
        // Query /addVaccinationRecords
        else if(tokens_ptr[0][0]=="/addVaccinationRecords")
        {
            if(tokens!=2)
                std::cout<<"ERROR: /addVaccinationRecords shall be called with exactly 1 argument."<<std::endl;
            else if(comm_handler::val_flc(tokens_ptr[1][0])==-1)
                std::cout<<"ERROR: Some arguments are not in valid format."<<std::endl;
            else
                addVaccinationRecords(tokens_ptr[1][0]);
        }
        // Query /travelStats
        else if(tokens_ptr[0][0]=="/travelStats")
        {
            if(tokens<4 || tokens>5)
                std::cout<<"ERROR: /travelStats shall be called with 3 or 4 arguments."<<std::endl;
            else if(comm_handler::val_virus(tokens_ptr[1][0])==-1 || comm_handler::val_date(tokens_ptr[2][0])==-1 || comm_handler::val_date(tokens_ptr[3][0])==-1)
                std::cout<<"ERROR: Some arguments are not in valid format."<<std::endl;
            else if(tokens==4)
                travelStats_noarg(tokens_ptr[1][0], tokens_ptr[2][0], tokens_ptr[3][0]);
            else if(comm_handler::val_flc(tokens_ptr[4][0])==-1)
                std::cout<<"ERROR: Some arguments are not in valid format."<<std::endl;
            else
                travelStats_arg(tokens_ptr[1][0], tokens_ptr[2][0], tokens_ptr[3][0],tokens_ptr[4][0]);
        }
        // Query /exit
        else if(tokens_ptr[0][0]=="/exit")
        {
            comm_handler::delete_tokens(tokens_ptr);
            ext();
        }
        else
            std::cout<<"ERROR: Command not recognized."<<std::endl;

        if(SIGINT_SIGQUIT_pending)
            raise(SIGINT);

        comm_handler::delete_tokens(tokens_ptr);
    }
}











////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int mst::delete_files()
{
    // Υπολογισε τα βασικα ονοματα των parent και child FIFOs
    std::string p_fifo=fifo_name+FIFO_P_SUFFIX;
    std::string c_fifo=fifo_name+FIFO_C_SUFFIX;
    // Για καθε εργατη
    for(unsigned int i=0;i<mat_size;i++)
    {
        // Παρε τους fds και το process number του
        int read_fd=workersptr[i].get_read_fd();
        int write_fd=workersptr[i].get_write_fd();
        int pr_number=workersptr[i].get_pr_number();
        // Κλεισε τους ανοιχτους read και write file descriptors
        close(read_fd);
        close(write_fd);
        // Αφαιρεσε τους απ' τα read και write sets
        del_fd_rd(read_fd);
        del_fd_wr(write_fd);
        // Υπολογισε τα πληρη ονοματα των FIFOs
        std::string p_extended=p_fifo+std::to_string(pr_number);
        std::string c_extended=c_fifo+std::to_string(pr_number);
        // Σβησε τα 2 FIFOs
        unlink(p_extended.c_str());
        unlink(c_extended.c_str());
    }
    return 0;
}

void mst::free_mem()
{
    delete_files();
    closedir(dir_ptr);
    for(unsigned int i=0;i<mat_size;i++)
    {
        std::string** str_ptr=workersptr[i].get_dirs_ptr();
        comm_handler::delete_tokens(str_ptr);
    }
    delete[] workersptr;
    delete virusesptr;
}



mst::~mst()
{
    delete_files();
    closedir(dir_ptr);
    for(unsigned int i=0;i<mat_size;i++)
    {
        std::string** str_ptr=workersptr[i].get_dirs_ptr();
        comm_handler::delete_tokens(str_ptr);
    }
    delete[] workersptr;
    delete virusesptr;
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Αλλες συναρτησεις

bool mst::error_check(unsigned int mask, unsigned int error)
{
    return (mask & error)==error;
}


void mst::print_workers()
{
    std::cout<<"|---------------------------------------------------|"<<std::endl;
    for(unsigned int i=0;i<mat_size;i++)
    {
        std::cout<<"PID: "<<workersptr[i].get_pid();
        std::cout<<"  --  Pr_Num: "<<workersptr[i].get_pr_number();
        std::cout<<"  --  Write_FD: "<<workersptr[i].get_write_fd();
        std::cout<<"  --  Read_FD: "<<workersptr[i].get_read_fd()<<endl;
        std::cout<<"Dir_Names: "<<workersptr[i].get_dirs();
        std::cout<<std::endl;
    }
    std::cout<<"|---------------------------------------------------|"<<std::endl;
}

int mst::find_pvt(std::string* matrix, int first, int last)
{
    std::string pivotElement=matrix[last];
    int pivotPosition=first-1;
    for(int j=first;j<last;j++)
    {
        if(matrix[j]<pivotElement)
        {
            pivotPosition++;
            std::string temp=matrix[pivotPosition];
            matrix[pivotPosition]=matrix[j];
            matrix[j]=temp;
        }
    }
    std::string temp=matrix[pivotPosition+1];
    matrix[pivotPosition+1]=matrix[last];
    matrix[last]=temp;
    return (pivotPosition+1);
}

void mst::sort_dirs(std::string *matrix, int first, int last)
{
    if (first<last)
    {
        int pivot=find_pvt(matrix, first, last);
        sort_dirs(matrix, first, pivot-1);
        sort_dirs(matrix, pivot+1, last);
    }
}
